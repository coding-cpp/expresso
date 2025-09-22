#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace core {

class Router {
private:
  std::map<std::string, void (*)(expresso::messages::Request& request,
                                 expresso::messages::Response& response)>
  getMap;
  std::map<std::string, void (*)(expresso::messages::Request& request,
                                 expresso::messages::Response& response)>
  postMap;
  std::map<std::string, void (*)(expresso::messages::Request& request,
                                 expresso::messages::Response& response)>
  putMap;
  std::map<std::string, void (*)(expresso::messages::Request& request,
                                 expresso::messages::Response& response)>
  patchMap;
  std::map<std::string, void (*)(expresso::messages::Request& request,
                                 expresso::messages::Response& response)>
  deleteMap;
  std::map<std::string, void (*)(expresso::messages::Request& request,
                                 expresso::messages::Response& response)>
  optionsMap;
  std::map<std::string, Router*> routerMap;

  Router* paramRouter;
  std::string paramRouterParam;

  std::vector<std::unique_ptr<expresso::middleware::Middleware>> middlewares;

  bool handleMiddlewares(expresso::messages::Request& request,
                         expresso::messages::Response& response);
  std::map<std::string, void (*)(expresso::messages::Request& request,
                                 expresso::messages::Response& response)>&
  fetchMapFromMethod(mochios::enums::method method);
  void addRoute(mochios::enums::method method, std::string path,
                void (*handler)(expresso::messages::Request& request,
                                expresso::messages::Response& response));

public:
  Router();
  ~Router();

  void get(std::string path,
           void (*handler)(expresso::messages::Request& request,
                           expresso::messages::Response& response));
  void post(std::string path,
            void (*handler)(expresso::messages::Request& request,
                            expresso::messages::Response& response));
  void put(std::string path,
           void (*handler)(expresso::messages::Request& request,
                           expresso::messages::Response& response));
  void patch(std::string path,
             void (*handler)(expresso::messages::Request& request,
                             expresso::messages::Response& response));
  void del(std::string path,
           void (*handler)(expresso::messages::Request& request,
                           expresso::messages::Response& response));
  void options(std::string path,
               void (*handler)(expresso::messages::Request& request,
                               expresso::messages::Response& response));

  void use(std::string path, Router* router);
  void use(std::unique_ptr<expresso::middleware::Middleware> middleware);

  void handleRequest(expresso::messages::Request& request,
                     expresso::messages::Response& response);
};

} // namespace core

} // namespace expresso