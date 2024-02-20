#pragma once

#include <cstring>
#include <functional>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include <nexus/pool.h>

#include <expresso/core/router.h>
#include <expresso/middleware/middleware.h>
#include <expresso/version.h>

namespace expresso {

namespace core {

class Server : public Router {
private:
  int socket;
  size_t maxConnections;
  struct sockaddr_in address;

  std::set<middleware::Middleware *> middlewares;

  void acceptConnections();
  void handleConnection(int clientSocket);
  bool handleMiddlewares(core::Request &req, core::Response &res);

  Request makeRequest(std::string &request);

  nexus::pool threadPool;

public:
  Server(size_t maxConnections = 5, size_t maxThreads = 2);
  ~Server();

  void use(middleware::Middleware *middleware);
  using Router::use;

  void listen(int port, std::function<void()> callback = nullptr);
};

} // namespace core

} // namespace expresso