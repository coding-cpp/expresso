#include <expresso/core/router.h>

expresso::core::Router::Router() {
  this->paramRouter = nullptr;

  return;
}

expresso::core::Router::~Router() {
  if (this->paramRouter != nullptr) {
    delete this->paramRouter;
  }

  for (const std::pair<std::string, Router *> &x : this->routerMap) {
    delete x.second;
  }
  this->routerMap.clear();

  this->getMap.clear();
  this->putMap.clear();
  this->postMap.clear();
  this->patchMap.clear();
  this->deleteMap.clear();
  this->optionsMap.clear();

  return;
}

void expresso::core::Router::del(std::string path,
                                 void (*handler)(Request &request,
                                                 Response &response)) {
  if (path[0] != '/') {
    utils::print::error(
        "Router path must start with a '/', given: " + path,
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
    utils::print::error(
        "Router path must start with a '/', given: " + path,
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
    utils::print::error(
        "Router path must start with a '/', given: " + path,
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
    utils::print::error(
        "Router path must start with a '/', given: " + path,
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
    utils::print::error(
        "Router path must start with a '/', given: " + path,
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
    utils::print::error(
        "Router path must start with a '/', given: " + path,
        "void expresso::core::Router::put(std::string path, void "
        "(*handler)(Request &request, Response &response))");
    return;
  }

  this->putMap[path.substr(1, path.size())] = handler;
  return;
}

void expresso::core::Router::use(std::string path, Router *router) {
  if (path[0] != '/') {
    utils::print::error(
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

void expresso::core::Router::handleRequest(Request &request,
                                           Response &response) {
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
             this->deleteMap.find(request.path) != this->deleteMap.end()) {
    this->deleteMap[request.path](request, response);
  } else if (request.method == "OPTIONS" &&
             this->optionsMap.find(request.path) != this->optionsMap.end()) {
    this->optionsMap[request.path](request, response);
  }

  for (const std::pair<std::string, Router *> &x : this->routerMap) {
    if (request.path.size() < x.first.size())
      continue;

    if ((request.path + "/")
            .compare(0, (x.first + "/").size(), (x.first + "/")) == 0) {
      if (request.path == x.first) {
        request.path = "/";
      } else {
        request.path =
            request.path.substr(x.first.size() + 1, request.path.size());
      }
      x.second->handleRequest(request, response);
      return;
    }
  }

  if (this->paramRouter != nullptr) {
    std::string temp = request.path.substr(0, request.path.find('/'));
    request.params[this->paramRouterParam] = url_decode(temp);

    if (request.path.find('/') == std::string::npos) {
      request.path = "";
    } else {
      request.path =
          request.path.substr(request.path.find('/') + 1, request.path.size());
    }

    this->paramRouter->handleRequest(request, response);
    return;
  }

  response.status(StatusCode::NOT_FOUND).send("Not Found");

  return;
}
