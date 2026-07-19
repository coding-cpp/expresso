#include <cerrno>
#include <sstream>

#include <brewtils/sys.h>
#include <brewtils/url.h>

#include <expresso/core/io/socket.h>
#include <expresso/core/server.h>
#include <expresso/helpers/request.h>

expresso::core::Server::Server(size_t maxConnections, size_t maxThreads)
  : maxConnections(maxConnections), threadPool(maxThreads) {
  signal(SIGPIPE, SIG_IGN);
  brewtils::sys::exitIf(SIGINT);
  brewtils::sys::exitIf(SIGTERM);
  brewtils::sys::exitIf(SIGKILL);

  logger::info("Using expresso v" + std::to_string(EXPRESSO_VERSION_MAJOR) +
               "." + std::to_string(EXPRESSO_VERSION_MINOR) + "." +
               std::to_string(EXPRESSO_VERSION_PATCH));

  this->serverSocket = brewtils::sys::socket(AF_INET, SOCK_STREAM, 0);
  if (this->serverSocket < 0) {
    logger::error("Socket not created!", "expresso::core::Server::Server()");
  }

  int reuseAddress = 1;
  setsockopt(this->serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
             sizeof(reuseAddress));
  expresso::core::io::setNonBlocking(this->serverSocket);

  if (pipe(this->wakeupPipe) < 0) {
    logger::error("Wakeup pipe not created!",
                  "expresso::core::Server::Server()");
  }
  expresso::core::io::setNonBlocking(this->wakeupPipe[0]);
  expresso::core::io::setNonBlocking(this->wakeupPipe[1]);

  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;
  this->setupMiddlewares();

  return;
}

expresso::core::Server::~Server() {
  if (this->serverSocket > 0) {
    close(this->serverSocket);
  }
  close(this->wakeupPipe[0]);
  close(this->wakeupPipe[1]);

  return;
}

void expresso::core::Server::listen(int port, std::function<void()> callback) {
  this->address.sin_port = htons(port);
  pid_t pid = getpid();
  logger::info("Server started with PID " + std::to_string(pid));

  if (brewtils::sys::bind(this->serverSocket, (struct sockaddr*)&this->address,
                          sizeof(this->address)) < 0) {
    logger::error("Unable to bind socket!",
                  "void expresso::core::Server::listen(int port)");
  }

  if (brewtils::sys::listen(this->serverSocket, this->maxConnections) < 0) {
    logger::error("Unable to listen on socket!",
                  "void expresso::core::Server::listen(int port)");
  }

  if (callback != nullptr) {
    callback();
  }
  this->runEventLoop();

  return;
}

mochios::enums::method expresso::core::Server::getMethodFromString(
    const std::string& method) noexcept(false) {
  if (method == "GET") return mochios::enums::method::GET;
  else if (method == "POST") return mochios::enums::method::POST;
  else if (method == "PUT") return mochios::enums::method::PUT;
  else if (method == "PATCH") return mochios::enums::method::PATCH;
  else if (method == "DELETE") return mochios::enums::method::DELETE;
  else if (method == "OPTIONS") return mochios::enums::method::OPTIONS;
  else if (method == "HEAD") return mochios::enums::method::HEAD;
  else
    logger::error("Unsupported HTTP method: " + method,
                  "expresso::core::Server::getMethodFromString(const "
                  "std::string &method) noexcept(false)");
}

void expresso::core::Server::setupMiddlewares() {
  this->use(std::make_unique<expresso::middleware::Version>());
  this->use(std::make_unique<expresso::middleware::Date>());
}

void expresso::core::Server::runEventLoop() {
  this->poller.add(this->serverSocket,
                   expresso::enums::NetworkEventType::READ);
  this->poller.add(this->wakeupPipe[0],
                   expresso::enums::NetworkEventType::READ);

  std::chrono::steady_clock::time_point lastSweep =
      std::chrono::steady_clock::now();

  while (true) {
    for (const expresso::core::io::Event& event :
         this->poller.wait(Server::POLL_INTERVAL_MS)) {
      if (event.fd == this->serverSocket) {
        this->acceptConnections();
        continue;
      }
      if (event.fd == this->wakeupPipe[0]) {
        this->flushProcessedConnections();
        continue;
      }

      std::unordered_map<
          int, std::shared_ptr<expresso::core::io::Connection>>::iterator it =
          this->connections.find(event.fd);
      if (it == this->connections.end()) {
        continue;
      }
      if (event.type == expresso::enums::NetworkEventType::READ) {
        this->handleReadable(it->second);
      } else {
        this->handleWritable(it->second);
      }
    }

    // Drain regardless of wakeup-pipe events so a lost wakeup can only delay
    // a response by one poll interval, never strand it
    this->flushProcessedConnections();

    std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();
    if (now - lastSweep >= std::chrono::seconds(1)) {
      this->closeIdleConnections();
      lastSweep = now;
    }
  }

  return;
}

void expresso::core::Server::acceptConnections() {
  while (true) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(this->serverSocket,
                              (struct sockaddr*)&clientAddress,
                              &clientAddressLength);
    if (clientSocket < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        logger::error("Client connection not accepted!");
      }
      return;
    }

    // A setup failure must not unwind the event loop; drop the client and
    // keep serving
    try {
      std::shared_ptr<expresso::core::io::Connection> connection =
          std::make_shared<expresso::core::io::Connection>(clientSocket);
      this->connections[clientSocket] = connection;
      this->poller.add(clientSocket, expresso::enums::NetworkEventType::READ);
    } catch (const std::exception& e) {
      logger::error(e.what());
      // If the connection made it into the map, erasing it closes the socket
      // through the destructor; otherwise the raw fd is still ours to close
      if (this->connections.erase(clientSocket) == 0) {
        close(clientSocket);
      }
    }
  }

  return;
}

void expresso::core::Server::handleReadable(
    std::shared_ptr<expresso::core::io::Connection> connection) {
  if (connection->getPhase() !=
      expresso::core::io::Connection::Phase::READING) {
    return;
  }

  expresso::core::io::Connection::IOResult result = connection->read();
  if (result == expresso::core::io::Connection::IOResult::CLOSED) {
    this->closeConnection(connection);
    return;
  }

  if (!expresso::helpers::isRequestComplete(connection->getRequest())) {
    return;
  }

  // The worker owns the connection from here on; stop watching it until the
  // response is ready to be written
  this->poller.remove(connection->getSocket());
  this->processingConnections.insert(connection->getSocket());
  connection->setPhase(expresso::core::io::Connection::Phase::PROCESSING);
  this->threadPool.enqueue(
      [this, connection]() { this->processRequest(connection); });

  return;
}

void expresso::core::Server::processRequest(
    std::shared_ptr<expresso::core::io::Connection> connection) {
  expresso::messages::Response res;

  try {
    expresso::messages::Request req =
        this->makeRequest(connection->getRequest());
    req.res = &res;
    this->handleRequest(req, res);
  } catch (const std::exception& e) {
    logger::error(e.what());
    res.status(expresso::enums::STATUS_CODE::BAD_REQUEST).send("Bad Request");
  }

  res.end();
  connection->setResponse(res.takeData(), res.takeFileRegion());

  {
    std::lock_guard<std::mutex> lock(this->processedConnectionsMutex);
    this->processedConnections.push(connection->getSocket());
  }
  char wakeupSignal = 1;
  if (write(this->wakeupPipe[1], &wakeupSignal, 1) < 0) {
    // The pipe is full, so a wakeup is already pending and the event loop
    // will drain the whole queue
  }

  return;
}

void expresso::core::Server::flushProcessedConnections() {
  char buffer[64];
  while (read(this->wakeupPipe[0], buffer, sizeof(buffer)) > 0) {
  }

  std::queue<int> ready;
  {
    std::lock_guard<std::mutex> lock(this->processedConnectionsMutex);
    std::swap(ready, this->processedConnections);
  }

  while (!ready.empty()) {
    std::unordered_map<
        int, std::shared_ptr<expresso::core::io::Connection>>::iterator it =
        this->connections.find(ready.front());
    this->processingConnections.erase(ready.front());
    ready.pop();
    if (it != this->connections.end()) {
      try {
        this->poller.add(it->first, expresso::enums::NetworkEventType::WRITE);
      } catch (const std::exception& e) {
        logger::error(e.what());
        this->closeConnection(it->second);
      }
    }
  }

  return;
}

void expresso::core::Server::closeIdleConnections() {
  std::vector<std::shared_ptr<expresso::core::io::Connection>> idle;
  for (const std::pair<
           const int, std::shared_ptr<expresso::core::io::Connection>>& entry :
       this->connections) {
    if (this->processingConnections.find(entry.first) !=
        this->processingConnections.end()) {
      continue;
    }
    if (entry.second->idleFor(Server::IDLE_TIMEOUT)) {
      idle.push_back(entry.second);
    }
  }

  for (const std::shared_ptr<expresso::core::io::Connection>& connection :
       idle) {
    this->closeConnection(connection);
  }

  return;
}

void expresso::core::Server::handleWritable(
    std::shared_ptr<expresso::core::io::Connection> connection) {
  if (connection->getPhase() !=
      expresso::core::io::Connection::Phase::WRITING) {
    return;
  }

  expresso::core::io::Connection::IOResult result = connection->write();
  if (result == expresso::core::io::Connection::IOResult::PENDING) {
    return;
  }
  this->closeConnection(connection);

  return;
}

void expresso::core::Server::closeConnection(
    std::shared_ptr<expresso::core::io::Connection> connection) {
  this->poller.remove(connection->getSocket());
  this->connections.erase(connection->getSocket());

  return;
}

expresso::messages::Request expresso::core::Server::makeRequest(
    const std::string& request) noexcept(false) {
  std::string line;
  std::istringstream stream(request);
  std::getline(stream, line);

  std::vector<std::string> parts = brewtils::string::split(line, " ");
  if (parts.size() < 3) {
    logger::error("Malformed request line: " + line,
                  "expresso::core::Server::makeRequest(const std::string "
                  "&request) noexcept(false)");
  }
  const std::string method = brewtils::string::upper(parts[0]);
  expresso::messages::Request req(parts[1]);
  req.method = this->getMethodFromString(method);
  req.httpVersion = parts[2];
  if (req.httpVersion.substr(0, 5) != "HTTP/") {
    logger::error("Invalid HTTP version: " + req.httpVersion,
                  "expresso::core::Server::makeRequest(const std::string "
                  "&request) noexcept(false)");
  }

  req.tempPath = req.path.substr(1, req.path.size());

  // Headers
  while (std::getline(stream, line) && line != "\r") {
    size_t separator = line.find(':', 0);
    if (separator != std::string::npos) {
      std::string key = brewtils::string::lower(line.substr(0, separator));
      std::string value =
          line.substr(separator + 2, line.size() - separator - 3);
      if (key == "content-length") {
        req.contentLength = std::stoi(value);
      } else if (key == "host") {
        req.host = value;
      } else if (key == "x-requested-with" && value == "XMLHttpRequest") {
        req.xhr = true;
      } else if (key == "origin") {
        if (value.size() > 7 && value.substr(0, 7) == "http://") {
          value = value.substr(7, value.size());
        } else if (value.size() > 8 && value.substr(0, 8) == "https://") {
          value = value.substr(8, value.size());
        }
        req.headers[key] = value.substr(0, value.find(":", 0));
        continue;
      }

      req.headers[key] = value;
    }
  }

  // Queries
  size_t start = req.tempPath.find('?', 0);
  if (start != std::string::npos) {
    start += 1;
    while (start < req.tempPath.size()) {
      size_t separator = req.tempPath.find('=', start);
      if (separator != std::string::npos) {
        std::string key = req.tempPath.substr(start, separator - start);
        start = separator + 1;
        size_t end = req.tempPath.find('&', start);
        if (end == std::string::npos) {
          end = req.tempPath.size();
        }

        std::string value = req.tempPath.substr(start, end - start);
        req.queries[brewtils::url::decode(key)] = brewtils::url::decode(value);
        start = end + 1;
      }
    }
  }
  req.path = req.tempPath;

  // Fixing the tempPath
  req.tempPath = req.tempPath.substr(0, req.tempPath.find('?', 0));
  if (!req.tempPath.empty() &&
      req.tempPath[req.tempPath.size() - 1] == '/') {
    req.tempPath = req.tempPath.substr(0, req.tempPath.size() - 1);
  }

  if (req.method == mochios::enums::method::GET ||
      req.method == mochios::enums::method::HEAD) {
    return req;
  }

  // Setting the body
  std::string contentType = req.headers["content-type"];
  std::string body = request.substr(request.find("\r\n\r\n") + 4);
  if (contentType == "text/plain" || contentType == "application/javascript") {
    req.body = json::object(body);
  } else if (contentType == "application/json") {
    json::parser parser;
    req.body = parser.loads(body);
  } else if (contentType == "application/x-www-form-urlencoded") {
    std::vector<std::string> parts = brewtils::string::split(body, "&");
    std::string key;
    std::string value;
    req.body = json::object(std::map<std::string, json::object>());
    for (const std::string& str : parts) {
      std::vector<std::string> pair = brewtils::string::split(str, "=");
      if (pair.size() != 2) {
        continue;
      }
      key = brewtils::url::decode(pair[0]);
      value = brewtils::url::decode(pair[1]);
      req.body[key] = json::object(value);
    }
  } else if (brewtils::string::split(contentType, ";")[0] ==
             "multipart/form-data") {
    std::vector<std::string> typeParts =
        brewtils::string::split(contentType, ";");
    if (typeParts.size() < 2) {
      return req;
    }
    std::vector<std::string> boundaryParts =
        brewtils::string::split(typeParts[1], "=");
    if (boundaryParts.size() < 2) {
      return req;
    }

    std::string delimiter = boundaryParts[1];
    std::vector<std::string> parts = brewtils::string::split(body, delimiter);
    std::vector<std::string> data;
    std::string key;
    std::string value;
    req.body = json::object(std::map<std::string, json::object>());
    for (const std::string& str : parts) {
      data = brewtils::string::split(str,
                                     "Content-Disposition: form-data; name=\"");
      if (data.size() == 2) {
        key = brewtils::string::split(data[1], "\r\n")[0];
        key = key.substr(0, key.size() - 1);
        std::vector<std::string> valueParts =
            brewtils::string::split(data[1], "\r\n\r\n");
        if (valueParts.size() < 2) {
          continue;
        }
        value = valueParts[1];
        value = brewtils::string::trim(value.substr(0, value.size() - 3));
        req.body[key] = json::object(value);
      }
    }
  }

  return req;
}
