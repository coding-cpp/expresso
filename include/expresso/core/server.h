#pragma once

#include <chrono>
#include <csignal>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

#include <json/parse.h>
#include <nexus/pool.h>

#include <expresso/core/io/connection.h>
#include <expresso/core/io/poller.h>
#include <expresso/core/router.h>
#include <expresso/middleware/date.h>
#include <expresso/middleware/version.h>
#include <expresso/version.h>

namespace expresso {

namespace core {

// Reactor-style server: a single event loop thread performs all socket i/o
// through non-blocking sockets and the platform poller, while request
// handling runs on the thread pool. Workers hand finished responses back to
// the event loop through `processedConnections` and the wakeup pipe, so no
// thread ever blocks on a client.
class Server : public Router {
private:
  int serverSocket;
  size_t maxConnections;
  struct sockaddr_in address;

  static constexpr std::chrono::seconds IDLE_TIMEOUT{60};
  static constexpr int POLL_INTERVAL_MS = 1000;

  expresso::core::io::Poller poller;
  std::unordered_map<int, std::shared_ptr<expresso::core::io::Connection>>
      connections;

  // Sockets currently owned by a worker thread. Only the event loop touches
  // this set; the idle sweep must skip these connections because their state
  // is being written by the worker.
  std::unordered_set<int> processingConnections;

  int wakeupPipe[2];
  std::queue<int> processedConnections;
  std::mutex processedConnectionsMutex;

  // Declared last so its destructor joins the workers while the members they
  // use are still alive
  nexus::pool threadPool;

  mochios::enums::method getMethodFromString(
      const std::string& method) noexcept(false);

  void setupMiddlewares();
  void runEventLoop();
  void acceptConnections();
  void handleReadable(
      std::shared_ptr<expresso::core::io::Connection> connection);
  void handleWritable(
      std::shared_ptr<expresso::core::io::Connection> connection);
  void processRequest(
      std::shared_ptr<expresso::core::io::Connection> connection);
  void flushProcessedConnections();
  void closeIdleConnections();
  void closeConnection(
      std::shared_ptr<expresso::core::io::Connection> connection);

  expresso::messages::Request makeRequest(
      const std::string& request) noexcept(false);

public:
  Server(size_t maxConnections = SOMAXCONN,
         size_t maxThreads = std::thread::hardware_concurrency());
  ~Server();

  void listen(int port, std::function<void()> callback = nullptr);
};

} // namespace core

} // namespace expresso
