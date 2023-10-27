#pragma once

#include <iostream>
#include <map>

#include <netinet/in.h>
#include <sys/socket.h>

#include <expresso/request.h>
#include <expresso/response.h>
#include <expresso/status_code.h>

class Router {
private:
  std::string basePath;
  std::string param;

  std::map<std::string, void (*)(Request &request, Response &response)> getMap;
  std::map<std::string, void (*)(Request &request, Response &response)> postMap;
  std::map<std::string, void (*)(Request &request, Response &response)> putMap;
  std::map<std::string, void (*)(Request &request, Response &response)>
      patchMap;
  std::map<std::string, void (*)(Request &request, Response &response)> delMap;
  std::map<std::string, void (*)(Request &request, Response &response)>
      optionsMap;

  Router *paramRouter = nullptr;

public:
  Router();
  ~Router();

  void setBasePath(std::string basePath);
  std::string getBasePath();

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

  void handleRequest(Request &request, Response &response);

  void setParamRouter(Router *router, std::string param);
};