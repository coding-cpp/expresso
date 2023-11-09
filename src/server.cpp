#include <expresso/server.h>

Server::Server() {
  this->socket = sys::socket_wrapper(AF_INET, SOCK_STREAM, 0);
  if (this->socket < 0) {
    std::cerr << "[ERROR] Socket not created!" << std::endl;
    exit(1);
  }

  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;

  return;
}

Server::~Server() {
  close(this->socket);

  return;
}

void Server::use(Router *router) {
  this->routerMap[router->getBasePath()] = *router;

  return;
}

void Server::run(int port) {
  this->address.sin_port = htons(port);

  if (bind(this->socket, (struct sockaddr *)&this->address,
           sizeof(this->address)) < 0) {
    std::cerr << "[ERROR] Unable to bind socket!" << std::endl;
    exit(1);
  }
  if (listen(this->socket, 5) < 0) {
    std::cerr << "[ERROR] Unable to listen on socket!" << std::endl;
    exit(1);
  }

  std::cout << "[INFO] Listening on port " << port << std::endl;
  this->acceptConnections();

  return;
}

void Server::acceptConnections() {
  while (true) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(this->socket, (struct sockaddr *)&clientAddress,
                              &clientAddressLength);

    if (clientSocket < 0) {
      std::cerr << "[ERROR] Client connection not accepted!" << std::endl;
      exit(1);
    }

    std::thread([this, clientSocket]() {
      this->handleConnection(clientSocket);
    }).detach();
  }

  return;
}

void Server::handleConnection(int clientSocket) {
  char charRequest[4096];
  memset(charRequest, 0, sizeof(charRequest));
  recv(clientSocket, charRequest, sizeof(charRequest) - 1, 0);

  std::string request = charRequest;

  char charMethod[16];
  char charPath[4096];
  sscanf(charRequest, "%s /%s HTTP/1.1", charMethod, charPath);

  std::string method = charMethod;
  std::string path = charPath;
  std::string routerPath = path.substr(0, path.find('/'));
  routerPath = routerPath.substr(0, routerPath.find('?'));

  if (this->routerMap.find(routerPath) != this->routerMap.end()) {
    Request req = this->makeRequest(request);
    Response res(clientSocket);
    this->routerMap[routerPath].handleRequest(req, res);
  } else {
    std::string response = "404 Not Found\r\n";

    std::string header = "HTTP/1.1 404 Not Found\r\n";
    header += "Content-Type: text/plain\r\n";
    header += "Content-Length: " + std::to_string(response.length()) + "\r\n";
    header += "\r\n";

    send(clientSocket, header.c_str(), header.length(), 0);
    send(clientSocket, response.c_str(), response.length(), 0);
  }

  close(clientSocket);

  return;
}

Request Server::makeRequest(std::string &request) {
  Request req;
  char method[16];
  char path[4096];
  sscanf(request.c_str(), "%s /%s HTTP/1.1", method, path);

  req.method = method;
  req.path = path;

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
      req.headers[key] = value;
    }

    start = end + 1;
  }

  // Params
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
        req.body.params[key] = value;

        start = end + 1;
      }
    }
  }

  req.path = req.path.substr(0, req.path.find('?'));
  if (req.path[req.path.size() - 1] == '/') {
    req.path = req.path.substr(0, req.path.size() - 1);
  }
  if (req.path.find('/') == std::string::npos) {
    req.path = "";
  } else {
    req.path = req.path.substr(req.path.find('/') + 1, req.path.size());
  }

  return req;
}