#pragma once

#include <chrono>
#include <string>

#include <expresso/messages/file_region.h>

namespace expresso {

namespace core {

namespace io {

// One client connection moving through its lifecycle:
// READING (event loop) -> PROCESSING (worker thread) -> WRITING (event loop).
// The event loop and a worker never own a connection at the same time, so no
// synchronization is needed on its members.
class Connection {
public:
  enum class Phase {
    READING,
    PROCESSING,
    WRITING,
  };

  enum class IOResult {
    PENDING,
    COMPLETE,
    CLOSED,
  };

private:
  int socket;
  Phase phase;

  std::string readBuffer;
  std::string writeBuffer;
  size_t bytesWritten;
  expresso::messages::FileRegion file;
  std::chrono::steady_clock::time_point lastActivity;

  IOResult flushBuffer();
  IOResult flushFile();

public:
  explicit Connection(int socket) noexcept(false);
  ~Connection();

  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  int getSocket() const;
  Phase getPhase() const;
  void setPhase(Phase phase);

  const std::string& getRequest() const;
  void setResponse(std::string&& data, expresso::messages::FileRegion file);

  bool idleFor(std::chrono::seconds duration) const;

  IOResult read();
  IOResult write();
};

} // namespace io

} // namespace core

} // namespace expresso
