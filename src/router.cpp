#include <expresso/router.h>

Router::Router() { return; }

Router::~Router() { return; }

void Router::setBasePath(std::string basePath) {
  if (basePath[0] != '/') {
    std::cout << "[ERROR] Router base path must start with a '/'" << std::endl;
    return;
  }

  this->basePath = basePath.substr(1, basePath.length());

  return;
}

std::string Router::getBasePath() { return this->basePath; }

void Router::get(std::string path, void (*handler)(int clientSocket)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->getMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::post(std::string path, void (*handler)(int clientSocket)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->postMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::put(std::string path, void (*handler)(int clientSocket)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->putMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::patch(std::string path, void (*handler)(int clientSocket)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->patchMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::del(std::string path, void (*handler)(int clientSocket)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->delMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::options(std::string path, void (*handler)(int clientSocket)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->optionsMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::handleRequest(int clientSocket, std::string method,
                           std::string path) {
  if (path[path.size() - 1] == '/') {
    path = path.substr(0, path.size() - 1);
  }

  if (method == "GET" && this->getMap.find(path) != this->getMap.end()) {
    this->getMap[path](clientSocket);
  } else if (method == "POST" &&
             this->postMap.find(path) != this->postMap.end()) {
    this->postMap[path](clientSocket);
  } else if (method == "PUT" && this->putMap.find(path) != this->putMap.end()) {
    this->putMap[path](clientSocket);
  } else if (method == "PATCH" &&
             this->patchMap.find(path) != this->patchMap.end()) {
    this->patchMap[path](clientSocket);
  } else if (method == "DELETE" &&
             this->delMap.find(path) != this->delMap.end()) {
    this->delMap[path](clientSocket);
  } else if (method == "OPTIONS" &&
             this->optionsMap.find(path) != this->optionsMap.end()) {
    this->optionsMap[path](clientSocket);
  } else {
    std::string response = "404 Not Found";

    std::string header = "HTTP/1.1 404 Not Found\r\n";
    header += "Content-Type: text/plain\r\n";
    header += "Content-Length: " + std::to_string(response.length()) + "\r\n";
    header += "\r\n";

    send(clientSocket, header.c_str(), header.length(), 0);
    send(clientSocket, response.c_str(), response.length(), 0);
  }

  return;
}