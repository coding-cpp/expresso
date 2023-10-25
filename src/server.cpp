#include <expresso/server.h>

Server::Server() {
  this->_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (this->_socket < 0) {
    std::cerr << "[ERROR] Socket not created!" << std::endl;
    exit(1);
  }

  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;

  return;
}

Server::~Server() {
  close(this->_socket);

  return;
}

void Server::use(Router *router) {
  this->routerMap[router->getBasePath()] = *router;

  return;
}

void Server::run(int port) {
  this->address.sin_port = htons(port);

  if (bind(this->_socket, (struct sockaddr *)&this->address,
           sizeof(this->address)) < 0) {
    std::cerr << "[ERROR] Unable to bind socket!" << std::endl;
    exit(1);
  }
  if (listen(this->_socket, 5) < 0) {
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
    int clientSocket = accept(this->_socket, (struct sockaddr *)&clientAddress,
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
  char request[4096];
  memset(request, 0, sizeof(request));
  recv(clientSocket, request, sizeof(request) - 1, 0);

  char charMethod[16];
  char charPath[4096];
  sscanf(request, "%s /%s HTTP/1.1", charMethod, charPath);

  std::string method = charMethod;
  std::string path = charPath;
  std::string routerPath = path.substr(0, path.find('/'));

  if (this->routerMap.find(routerPath) != this->routerMap.end()) {
    this->routerMap[routerPath].handleRequest(
        clientSocket, method, path.substr(routerPath.length(), path.length()));
  } else {
    std::string response = "404 Not Found";

    std::string header = "HTTP/1.1 404 Not Found\r\n";
    header += "Content-Type: text/plain\r\n";
    header += "Content-Length: " + std::to_string(response.length()) + "\r\n";
    header += "\r\n";

    send(clientSocket, header.c_str(), header.length(), 0);
    send(clientSocket, response.c_str(), response.length(), 0);
  }

  close(clientSocket);

  //   if (router.getMap.find("/") != router.getMap.end()) {
  //     router.getMap["/"](clientSocket);
  //   } else {
  //     std::string response = "404 Not Found";

  //     std::string header = "HTTP/1.1 404 Not Found\r\n";
  //     header += "Content-Type: text/plain\r\n";
  //     header += "Content-Length: " + std::to_string(response.length()) +
  //     "\r\n"; header += "\r\n";

  //     send(clientSocket, header.c_str(), header.length(), 0);
  //     send(clientSocket, response.c_str(), response.length(), 0);
  //   }

  return;
}