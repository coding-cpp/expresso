#include <expresso/core/router.h>

expresso::core::Router::Router() : paramRouter(nullptr) { return; }

expresso::core::Router::~Router() {
  if (this->paramRouter != nullptr) {
    delete this->paramRouter;
  }

  for (const std::pair<std::string, Router *> &router : this->routerMap) {
    delete router.second;
  }

  this->routerMap.clear();
  this->getMap.clear();
  this->postMap.clear();
  this->putMap.clear();
  this->patchMap.clear();
  this->deleteMap.clear();
  this->optionsMap.clear();

  return;
}

void expresso::core::Router::del(std::string path,
                                 void (*handler)(Request &request,
                                                 Response &response)) {
  if (path[0] != '/') {
    logger::error("Router path must start with a '/', given: " + path,
                  "void expresso::core::Router::del(std::string path, void "
                  "(*handler)(Request &request, Response &response))");
    return;
  }

  this->deleteMap[path.substr(1, path.size())] = handler;
  return;
}

void expresso::core::Router::get(std::string path,
                                 void (*handler)(Request &request,
                                                 Response &response)) {
  if (path[0] != '/') {
    logger::error("Router path must start with a '/', given: " + path,
                  "void expresso::core::Router::get(std::string path, void "
                  "(*handler)(Request &request, Response &response))");
    return;
  }

  this->getMap[path.substr(1, path.size())] = handler;
  return;
}

void expresso::core::Router::options(std::string path,
                                     void (*handler)(Request &request,
                                                     Response &response)) {
  if (path[0] != '/') {
    logger::error("Router path must start with a '/', given: " + path,
                  "void expresso::core::Router::options(std::string path, void "
                  "(*handler)(Request &request, Response &response))");
    return;
  }

  this->optionsMap[path.substr(1, path.size())] = handler;
  return;
}

void expresso::core::Router::patch(std::string path,
                                   void (*handler)(Request &request,
                                                   Response &response)) {
  if (path[0] != '/') {
    logger::error("Router path must start with a '/', given: " + path,
                  "void expresso::core::Router::patch(std::string path, void "
                  "(*handler)(Request &request, Response &response))");
    return;
  }

  this->patchMap[path.substr(1, path.size())] = handler;
  return;
}

void expresso::core::Router::post(std::string path,
                                  void (*handler)(Request &request,
                                                  Response &response)) {
  if (path[0] != '/') {
    logger::error("Router path must start with a '/', given: " + path,
                  "void expresso::core::Router::post(std::string path, void "
                  "(*handler)(Request &request, Response &response))");
    return;
  }

  this->postMap[path.substr(1, path.size())] = handler;
  return;
}

void expresso::core::Router::put(std::string path,
                                 void (*handler)(Request &request,
                                                 Response &response)) {
  if (path[0] != '/') {
    logger::error("Router path must start with a '/', given: " + path,
                  "void expresso::core::Router::put(std::string path, void "
                  "(*handler)(Request &request, Response &response))");
    return;
  }

  this->putMap[path.substr(1, path.size())] = handler;
  return;
}

void expresso::core::Router::use(std::string path, Router *router) {
  if (path[0] != '/') {
    logger::error(
        "Router path must start with a '/', given: " + path,
        "void expresso::core::Router::use(std::string path, Router *router)");
    return;
  }

  if (path[path.size() - 1] == '/') {
    path = path.substr(0, path.size() - 1);
  }

  this->routerMap[path.substr(1, path.size())] = router;
  return;
}

void expresso::core::Router::use(expresso::middleware::Middleware *middleware) {
  this->middlewares.push_back(middleware);
  return;
}

void expresso::core::Router::handleRequest(Request &request,
                                           Response &response) {
  if (!this->handleMiddlewares(request, response)) {
    return;
  }

  if (request.tempPath.size() > 0 && request.tempPath[0] == '/') {
    request.tempPath = request.tempPath.substr(1, request.tempPath.size());
  }

  if (request.method == "GET" &&
      this->getMap.find(request.tempPath) != this->getMap.end()) {
    this->getMap[request.tempPath](request, response);
    return;
  } else if (request.method == "POST" &&
             this->postMap.find(request.tempPath) != this->postMap.end()) {
    this->postMap[request.tempPath](request, response);
    return;
  } else if (request.method == "PUT" &&
             this->putMap.find(request.tempPath) != this->putMap.end()) {
    this->putMap[request.tempPath](request, response);
    return;
  } else if (request.method == "PATCH" &&
             this->patchMap.find(request.tempPath) != this->patchMap.end()) {
    this->patchMap[request.tempPath](request, response);
    return;
  } else if (request.method == "DELETE" &&
             this->deleteMap.find(request.tempPath) != this->deleteMap.end()) {
    this->deleteMap[request.tempPath](request, response);
    return;
  } else if (request.method == "OPTIONS" &&
             this->optionsMap.find(request.tempPath) !=
                 this->optionsMap.end()) {
    this->optionsMap[request.tempPath](request, response);
    return;
  }

  for (const std::pair<std::string, Router *> &x : this->routerMap) {
    if (request.tempPath.size() < x.first.size())
      continue;

    if ((request.tempPath + "/")
            .compare(0, (x.first + "/").size(), (x.first + "/")) == 0) {
      if (request.tempPath == x.first) {
        request.tempPath = "";
      } else {
        request.tempPath = request.tempPath.substr(x.first.size() + 1,
                                                   request.tempPath.size());
      }
      x.second->handleRequest(request, response);
      return;
    }
  }

  if (this->paramRouter != nullptr) {
    std::string temp = request.tempPath.substr(0, request.tempPath.find('/'));
    request.params[this->paramRouterParam] = brewtils::url::decode(temp);

    if (request.tempPath.find('/') == std::string::npos) {
      request.tempPath = "";
    } else {
      request.tempPath = request.tempPath.substr(request.tempPath.find('/') + 1,
                                                 request.tempPath.size());
    }

    this->paramRouter->handleRequest(request, response);
    return;
  }

  response.sendNotFound();
  return;
}

bool expresso::core::Router::handleMiddlewares(Request &request,
                                               Response &response) {
  for (expresso::middleware::Middleware *middleware : this->middlewares) {
    if (!middleware->use(request, response)) {
      response.end();
      return false;
    }
  }

  return true;
}