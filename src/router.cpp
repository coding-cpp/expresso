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

void Router::get(std::string path,
                 void (*handler)(Request &request, Response &response)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->getMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::post(std::string path,
                  void (*handler)(Request &request, Response &response)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->postMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::put(std::string path,
                 void (*handler)(Request &request, Response &response)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->putMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::patch(std::string path,
                   void (*handler)(Request &request, Response &response)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->patchMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::del(std::string path,
                 void (*handler)(Request &request, Response &response)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->delMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::options(std::string path,
                     void (*handler)(Request &request, Response &response)) {
  if (path[0] != '/') {
    std::cout << "[ERROR] Router path must start with a '/'" << std::endl;
    return;
  }
  this->optionsMap[path.substr(1, path.size())] = handler;

  return;
}

void Router::handleRequest(Request &request, Response &response) {
  if (request.method == "GET" &&
      this->getMap.find(request.path) != this->getMap.end()) {
    this->getMap[request.path](request, response);
  } else if (request.method == "POST" &&
             this->postMap.find(request.path) != this->postMap.end()) {
    this->postMap[request.path](request, response);
  } else if (request.method == "PUT" &&
             this->putMap.find(request.path) != this->putMap.end()) {
    this->putMap[request.path](request, response);
  } else if (request.method == "PATCH" &&
             this->patchMap.find(request.path) != this->patchMap.end()) {
    this->patchMap[request.path](request, response);
  } else if (request.method == "DELETE" &&
             this->delMap.find(request.path) != this->delMap.end()) {
    this->delMap[request.path](request, response);
  } else if (request.method == "OPTIONS" &&
             this->optionsMap.find(request.path) != this->optionsMap.end()) {
    this->optionsMap[request.path](request, response);
  } else if (this->paramRouter != nullptr) {
    request.body.ids[this->param] =
        request.path.substr(0, request.path.find('/'));
    if (request.path.find('/') == std::string::npos) {
      request.path = "";
    } else {
      request.path = request.path.substr(request.path.find('/') + 1,
                                         request.path.length());
    }
    this->paramRouter->handleRequest(request, response);
  } else {
    response.status(StatusCode::NOT_FOUND).json("Not Found");
  }

  return;
}

void Router::setParamRouter(Router *router, std::string param) {
  this->paramRouter = router;
  this->param = param;

  return;
}