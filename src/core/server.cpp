#include <expresso/core/server.h>

expresso::core::Server::Server() {
  utils::print::info("Using expresso v" +
                     std::to_string(EXPRESSO_VERSION_MAJOR) + "." +
                     std::to_string(EXPRESSO_VERSION_MINOR) + "." +
                     std::to_string(EXPRESSO_VERSION_PATCH));

  this->socket = sys::socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket < 0) {
    utils::print::error("Socket not created!",
                        "expresso::core::Server::Server()");
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

void expresso::core::Server::run(int port) {
  this->address.sin_port = htons(port);

  if (bind(this->socket, (struct sockaddr *)&this->address,
           sizeof(this->address)) < 0) {
    utils::print::error("Unable to bind socket!",
                        "void expresso::core::Server::run(int port)");
  }

  if (listen(this->socket, 10) < 0) {
    utils::print::error("Unable to listen on socket!",
                        "void expresso::core::Server::run(int port)");
  }

  utils::print::info("Listening on port " + std::to_string(port));
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
      utils::print::error("Client connection not accepted!",
                          "void expresso::core::Server::acceptConnections()");
    }

    std::thread([this, clientSocket]() {
      this->handleConnection(clientSocket);
    }).detach();
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
      utils::print::error(
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
  Response res(clientSocket);
  req.res = &res;

  if (!this->handleMiddlewares(req, res)) {
    return;
  }

  this->handleRequest(req, res);

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
  start = req.path.find('?', 0);
  if (start != std::string::npos) {
    start += 1;
    while (start < req.path.size()) {
      size_t separator = req.path.find('=', start);
      if (separator != std::string::npos) {
        std::string key = req.path.substr(start, separator - start);
        start = separator + 1;

        size_t end = req.path.find('&', start);
        if (end == std::string::npos) {
          end = req.path.size();
        }

        std::string value = req.path.substr(start, end - start);
        req.queries[utils::url_decode(key)] = utils::url_decode(value);

        start = end + 1;
      }
    }
  }

  req.path = req.path.substr(0, req.path.find('?', 0));
  if (req.path[req.path.size() - 1] == '/') {
    req.path = req.path.substr(0, req.path.size() - 1);
  }

  return req;
}