#pragma once

#include <vector>

#include <expresso/enums/network_event_type.h>

#if !defined(__linux__) && !defined(__APPLE__)
#error "expresso only supports linux (epoll) and macOS (kqueue)"
#endif

namespace expresso {

namespace core {

namespace io {

struct Event {
  int fd;
  expresso::enums::NetworkEventType type;
};

// Thin wrapper over the platform readiness API: epoll on linux, kqueue on
// macOS. The backend is picked at compile time (poller_epoll.cpp or
// poller_kqueue.cpp).
class Poller {
private:
  int pollerFd;

  static constexpr int MAX_EVENTS = 64;

public:
  Poller() noexcept(false);
  ~Poller();

  Poller(const Poller&) = delete;
  Poller& operator=(const Poller&) = delete;

  void add(int fd, expresso::enums::NetworkEventType type) noexcept(false);
  void remove(int fd);
  std::vector<Event> wait(int timeoutMs = -1);
};

} // namespace io

} // namespace core

} // namespace expresso
