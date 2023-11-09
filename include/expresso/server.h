#pragma once

#include <cstring>
#include <thread>
#include <unistd.h>

#include <expresso/router.h>

class Server {
private:
  int socket;
  struct sockaddr_in address;

  std::map<std::string, Router> routerMap;

  void acceptConnections();
  void handleConnection(int clientSocket);

  Request makeRequest(std::string &request);

public:
  Server();
  ~Server();

  void use(Router *router);
  void run(int port);
};