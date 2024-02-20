#include <expresso/core/server.h>

expresso::core::Server::Server(size_t maxConnections, size_t maxThreads)
    : maxConnections(maxConnections), threadPool(nexus::pool(maxThreads)) {
  signal(SIGPIPE, SIG_IGN);
  logger::success("Using expresso v" + std::to_string(EXPRESSO_VERSION_MAJOR) +
                  "." + std::to_string(EXPRESSO_VERSION_MINOR) + "." +
                  std::to_string(EXPRESSO_VERSION_PATCH));

  this->socket = sys::socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket < 0) {
    logger::error("Socket not created!", "expresso::core::Server::Server()");
  }

  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;

  return;
}

expresso::core::Server::~Server() {
  close(this->socket);

  return;
}

void expresso::core::Server::use(middleware::Middleware *middleware) {
  this->middlewares.insert(middleware);

  return;
}

void expresso::core::Server::listen(int port, std::function<void()> callback) {
  this->address.sin_port = htons(port);

  if (bind(this->socket, (struct sockaddr *)&this->address,
           sizeof(this->address)) < 0) {
    logger::error("Unable to bind socket!",
                  "void expresso::core::Server::run(int port)");
  }

  if (sys::listen(this->socket, this->maxConnections) < 0) {
    logger::error("Unable to listen on socket!",
                  "void expresso::core::Server::run(int port)");
  }

  if (callback != nullptr) {
    callback();
  }
  this->acceptConnections();

  return;
}

void expresso::core::Server::acceptConnections() {
  while (true) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(this->socket, (struct sockaddr *)&clientAddress,
                              &clientAddressLength);

    if (clientSocket < 0) {
      logger::error("Client connection not accepted!",
                    "void expresso::core::Server::acceptConnections()");
      return;
    }

    this->threadPool.enqueue(
        [this, clientSocket]() { this->handleConnection(clientSocket); });
  }

  return;
}

void expresso::core::Server::handleConnection(int clientSocket) {
  constexpr size_t bufferSize = 4096;
  std::vector<char> charRequest;
  charRequest.resize(bufferSize, '\0');

  size_t totalBytesRead = 0;
  size_t bytesRead;

  do {
    bytesRead = recv(clientSocket, charRequest.data() + totalBytesRead,
                     bufferSize - 1, 0);
    if (bytesRead == static_cast<ssize_t>(-1)) {
      logger::error(
          "Failed to receive data from client!",
          "void expresso::core::Server::handleConnection(int clientSocket)");
      close(clientSocket);
      return;
    }

    totalBytesRead += bytesRead;
    charRequest.resize(totalBytesRead + bufferSize, '\0');
  } while (bytesRead > 0 && charRequest[totalBytesRead - 1] != '\n');

  charRequest.resize(totalBytesRead);
  std::string request(charRequest.data());

  Request req = this->makeRequest(request);
  Response *res = new Response(clientSocket);
  req.res = res;

  if (!this->handleMiddlewares(req, *res)) {
    delete res;
    return;
  }

  this->handleRequest(req, *res);
  delete res;

  return;
}

bool expresso::core::Server::handleMiddlewares(core::Request &req,
                                               core::Response &res) {
  for (middleware::Middleware *middleware : this->middlewares) {
    if (!middleware->use(req, res)) {
      return false;
    }
  }

  return true;
}

expresso::core::Request
expresso::core::Server::makeRequest(std::string &request) {
  Request req;
  char method[16];
  char path[4096];
  sscanf(request.c_str(), "%s /%s HTTP/1.1", method, path);

  req.method = method;
  req.path = path;
  req.path = req.path == "HTTP/1.1" ? "/" : req.path;
  req.tempPath = req.path;
  req.path = "/" + req.path;

  // Headers
  size_t start = request.find('\n', 0) + 1;
  while (start < request.size()) {
    size_t end = request.find('\n', start);
    if (end == std::string::npos) {
      end = request.size();
    }

    size_t separator = request.find(':', start);
    if (separator != std::string::npos) {
      std::string key = request.substr(start, separator - start);
      std::string value = request.substr(separator + 2, end - separator - 2);

      if (key == "Content-Length") {
        req.contentLength = std::stoi(value);
      }

      if (key == "Host") {
        req.host = value;
      }

      if (key == "X-Requested-With" && value == "XMLHttpRequest") {
        req.xhr = true;
      }

      if (key == "Origin") {
        if (value.size() > 7 && value.substr(0, 7) == "http://") {
          value = value.substr(7, value.size());
        } else if (value.size() > 8 && value.substr(0, 8) == "https://") {
          value = value.substr(8, value.size());
        }
        req.headers[key] = value.substr(0, value.find(":", 0));
        start = end + 1;

        continue;
      }

      req.headers[key] = value;
    }

    start = end + 1;
  }

  // Queries
  start = req.tempPath.find('?', 0);
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
        req.queries[utils::urlDecode(key)] = utils::urlDecode(value);

        start = end + 1;
      }
    }
  }

  req.tempPath = req.tempPath.substr(0, req.tempPath.find('?', 0));
  if (req.tempPath[req.tempPath.size() - 1] == '/') {
    req.tempPath = req.tempPath.substr(0, req.tempPath.size() - 1);
  }

  return req;
}