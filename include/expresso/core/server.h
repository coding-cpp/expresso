#pragma once

#include <cstring>
#include <thread>
#include <unistd.h>

#include <expresso/core/router.h>

namespace expresso {

namespace core {

class Server : public Router {
private:
  int socket;
  struct sockaddr_in address;

  void acceptConnections();
  void handleConnection(int clientSocket);

  Request makeRequest(std::string &request);

public:
  Server();
  ~Server();

  void run(int port);
};

} // namespace core

} // namespace expresso