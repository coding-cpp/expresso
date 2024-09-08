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

void expresso::core::Router::get(std::string path,
                                 void (*handler)(Request &request,
                                                 Response &response)) {
  this->addRoute(expresso::enums::method::GET, path, handler);
  return;
}

void expresso::core::Router::post(std::string path,
                                  void (*handler)(Request &request,
                                                  Response &response)) {
  this->addRoute(expresso::enums::method::POST, path, handler);
  return;
}

void expresso::core::Router::put(std::string path,
                                 void (*handler)(Request &request,
                                                 Response &response)) {
  this->addRoute(expresso::enums::method::PUT, path, handler);
  return;
}

void expresso::core::Router::patch(std::string path,
                                   void (*handler)(Request &request,
                                                   Response &response)) {
  this->addRoute(expresso::enums::method::PATCH, path, handler);
  return;
}

void expresso::core::Router::del(std::string path,
                                 void (*handler)(Request &request,
                                                 Response &response)) {
  this->addRoute(expresso::enums::method::DELETE, path, handler);
  return;
}

void expresso::core::Router::options(std::string path,
                                     void (*handler)(Request &request,
                                                     Response &response)) {
  this->addRoute(expresso::enums::method::OPTIONS, path, handler);
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

void expresso::core::Router::use(
    std::unique_ptr<expresso::middleware::Middleware> middleware) {
  this->middlewares.push_back(std::move(middleware));
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

  std::map<std::string, void (*)(Request & request, Response & response)> &map =
      this->fetchMapFromMethod(request.method);
  if (map.find(request.tempPath) != map.end()) {
    return map[request.tempPath](request, response);
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
  for (const std::unique_ptr<expresso::middleware::Middleware> &middleware :
       this->middlewares) {
    if (!middleware->use(request, response)) {
      response.end();
      return false;
    }
  }

  return true;
}

std::map<std::string, void (*)(expresso::core::Request &request,
                               expresso::core::Response &response)> &
expresso::core::Router::fetchMapFromMethod(expresso::enums::method method) {
  switch (method) {
  case expresso::enums::method::GET:
    return this->getMap;
  case expresso::enums::method::POST:
    return this->postMap;
  case expresso::enums::method::PUT:
    return this->putMap;
  case expresso::enums::method::PATCH:

    return this->patchMap;
  case expresso::enums::method::DELETE:
    return this->deleteMap;
  case expresso::enums::method::OPTIONS:
    return this->optionsMap;
  default:
    logger::error("Invalid method: " + std::to_string(static_cast<int>(method)),
                  "std::map<std::string, void (*)(expresso::core::Request "
                  "&request, expresso::core::Response &response)> "
                  "&expresso::core::Router::fetchRouterFromMethod(expresso::"
                  "enums::method method)");
    return this->getMap;
  }
}

void expresso::core::Router::addRoute(expresso::enums::method method,
                                      std::string path,
                                      void (*handler)(Request &request,
                                                      Response &response)) {
  if (path[0] != '/') {
    logger::error("Router path must start with a '/', given: " + path,
                  "void expresso::core::Router::put(std::string path, void "
                  "(*handler)(Request &request, Response &response))");
    return;
  }

  std::map<std::string, void (*)(Request & request, Response & response)> &map =
      this->fetchMapFromMethod(method);
  map[path.substr(1, path.size())] = handler;
  return;
}