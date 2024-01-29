#pragma once

#include <cstring>
#include <functional>
#include <thread>
#include <unistd.h>

#include <expresso/core/router.h>
#include <expresso/middleware/middleware.h>
#include <expresso/version.h>

namespace expresso {

namespace core {

class Server : public Router {
private:
  int socket;
  size_t concurrency;
  struct sockaddr_in address;

  std::set<middleware::Middleware *> middlewares;

  void acceptConnections();
  void handleConnection(int clientSocket);
  bool handleMiddlewares(core::Request &req, core::Response &res);

  Request makeRequest(std::string &request);

public:
  Server(size_t concurrency = 5);
  ~Server();

  void use(middleware::Middleware *middleware);
  using Router::use;

  void listen(int port, std::function<void()> callback = nullptr);
};

} // namespace core

} // namespace expresso