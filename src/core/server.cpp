#include <expresso/core/server.h>

expresso::core::Server::Server(size_t maxConnections, size_t maxThreads)
    : maxConnections(maxConnections), threadPool(maxThreads) {
  signal(SIGPIPE, SIG_IGN);
  logger::success("Using expresso v" + std::to_string(EXPRESSO_VERSION_MAJOR) +
                  "." + std::to_string(EXPRESSO_VERSION_MINOR) + "." +
                  std::to_string(EXPRESSO_VERSION_PATCH));

  this->socket = brewtils::sys::socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket < 0) {
    logger::error("Socket not created!", "expresso::core::Server::Server()");
  }

  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;

  return;
}

expresso::core::Server::~Server() { return; }

void expresso::core::Server::listen(int port, std::function<void()> callback) {
  this->address.sin_port = htons(port);

  if (brewtils::sys::bind(this->socket, (struct sockaddr *)&this->address,
                          sizeof(this->address)) < 0) {
    logger::error("Unable to bind socket!",
                  "void expresso::core::Server::run(int port)");
  }

  if (brewtils::sys::listen(this->socket, this->maxConnections) < 0) {
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
    bytesRead = brewtils::sys::recv(
        clientSocket, charRequest.data() + totalBytesRead, bufferSize - 1, 0);
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
  res->set("Expresso", "v" + std::to_string(EXPRESSO_VERSION_MAJOR) + "." +
                           std::to_string(EXPRESSO_VERSION_MINOR) + "." +
                           std::to_string(EXPRESSO_VERSION_PATCH));
  req.res = res;
  this->handleRequest(req, *res);
  delete res;

  return;
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
  std::string line;
  std::istringstream stream(request);
  while (std::getline(stream, line) && line != "\r") {
    size_t separator = line.find(':', 0);
    if (separator != std::string::npos) {
      std::string key = line.substr(0, separator);
      std::string value =
          line.substr(separator + 2, line.size() - separator - 3);
      if (key == "Content-Length" || key == "content-length") {
        req.contentLength = std::stoi(value);
      } else if (key == "Host" || key == "host") {
        req.host = value;
      } else if ((key == "X-Requested-With" || key == "x-requested-with") &&
                 value == "XMLHttpRequest") {
        req.xhr = true;
      } else if (key == "Origin" || key == "origin") {
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
  if (req.tempPath[req.tempPath.size() - 1] == '/') {
    req.tempPath = req.tempPath.substr(0, req.tempPath.size() - 1);
  }

  // Setting the body
  std::string contentType = req.headers["Content-Type"];
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
    for (auto str : parts) {
      key = brewtils::url::decode(brewtils::string::split(str, "=")[0]);
      value = brewtils::url::decode(brewtils::string::split(str, "=")[1]);
      req.body[key] = json::object(value);
    }
  } else if (brewtils::string::split(contentType, ";")[0] ==
             "multipart/form-data") {
    std::string delimiter = brewtils::string::split(
        brewtils::string::split(contentType, ";")[1], "=")[1];
    std::vector<std::string> parts = brewtils::string::split(body, delimiter);
    std::vector<std::string> data;
    std::string key;
    std::string value;
    req.body = json::object(std::map<std::string, json::object>());
    for (auto str : parts) {
      data = brewtils::string::split(str,
                                     "Content-Disposition: form-data; name=\"");
      if (data.size() == 2) {
        key = brewtils::string::split(data[1], "\r\n")[0];
        key = key.substr(0, key.size() - 1);
        value = brewtils::string::split(data[1], "\r\n\r\n")[1];
        value = value.substr(0, value.size() - 3);
        req.body[key] = json::object(value);
      }
    }
  }

  return req;
}