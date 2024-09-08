#pragma once

#include <csignal>
#include <unistd.h>

#include <json/parse.h>
#include <nexus/pool.h>

#include <expresso/core/router.h>
#include <expresso/middleware/date.h>
#include <expresso/middleware/version.h>
#include <expresso/version.h>

namespace expresso {

namespace core {

class Server : public Router {
private:
  int socket;
  size_t maxConnections;
  struct sockaddr_in address;

  void setupMiddlewares();
  void acceptConnections();
  void handleConnection(int clientSocket);

  expresso::core::Request makeRequest(std::string &request) noexcept(false);
  nexus::pool threadPool;

public:
  Server(size_t maxConnections = SOMAXCONN,
         size_t maxThreads = std::thread::hardware_concurrency());
  ~Server();

  void listen(int port, std::function<void()> callback = nullptr);
};

} // namespace core

} // namespace expresso