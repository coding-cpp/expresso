#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace core {

class Router {
private:
  std::map<std::string, void (*)(Request &request, Response &response)> getMap;
  std::map<std::string, void (*)(Request &request, Response &response)> postMap;
  std::map<std::string, void (*)(Request &request, Response &response)> putMap;
  std::map<std::string, void (*)(Request &request, Response &response)>
      patchMap;
  std::map<std::string, void (*)(Request &request, Response &response)>
      deleteMap;
  std::map<std::string, void (*)(Request &request, Response &response)>
      optionsMap;
  std::map<std::string, Router *> routerMap;

  Router *paramRouter;
  std::string paramRouterParam;

  std::vector<expresso::middleware::Middleware *> middlewares;

  bool handleMiddlewares(Request &request, Response &response);

public:
  Router();
  ~Router();

  void get(std::string path,
           void (*handler)(Request &request, Response &response));
  void post(std::string path,
            void (*handler)(Request &request, Response &response));
  void put(std::string path,
           void (*handler)(Request &request, Response &response));
  void patch(std::string path,
             void (*handler)(Request &request, Response &response));
  void del(std::string path,
           void (*handler)(Request &request, Response &response));
  void options(std::string path,
               void (*handler)(Request &request, Response &response));

  void use(std::string path, Router *router);
  void use(expresso::middleware::Middleware *middleware);

  void handleRequest(Request &request, Response &response);
};

} // namespace core

} // namespace expresso