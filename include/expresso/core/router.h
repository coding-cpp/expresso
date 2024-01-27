#pragma once

#include <map>
#include <string>

#include <expresso/core/request.h>
#include <expresso/core/response.h>
#include <expresso/core/status_code.h>
#include <expresso/utils/print.h>
#include <expresso/utils/sterlize.h>

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
  void handleRequest(Request &request, Response &response);
};

} // namespace core

} // namespace expresso
