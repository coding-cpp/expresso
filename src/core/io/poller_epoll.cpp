#ifdef __linux__

#include <sys/epoll.h>
#include <unistd.h>

#include <logger/log.h>

#include <expresso/core/io/poller.h>

expresso::core::io::Poller::Poller() noexcept(false) {
  this->pollerFd = epoll_create1(0);
  if (this->pollerFd < 0) {
    logger::error("Unable to create epoll instance!",
                  "expresso::core::io::Poller::Poller()");
  }

  return;
}

expresso::core::io::Poller::~Poller() {
  if (this->pollerFd >= 0) {
    close(this->pollerFd);
  }

  return;
}

void expresso::core::io::Poller::add(
    int fd, expresso::enums::NetworkEventType type) noexcept(false) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events =
      type == expresso::enums::NetworkEventType::READ ? EPOLLIN : EPOLLOUT;
  if (epoll_ctl(this->pollerFd, EPOLL_CTL_ADD, fd, &event) < 0) {
    logger::error("Unable to watch fd " + std::to_string(fd) + "!",
                  "void expresso::core::io::Poller::add(int fd, "
                  "expresso::enums::NetworkEventType type)");
  }

  return;
}

void expresso::core::io::Poller::remove(int fd) {
  epoll_ctl(this->pollerFd, EPOLL_CTL_DEL, fd, nullptr);

  return;
}

std::vector<expresso::core::io::Event>
expresso::core::io::Poller::wait(int timeoutMs) {
  struct epoll_event events[Poller::MAX_EVENTS];
  int count = epoll_wait(this->pollerFd, events, Poller::MAX_EVENTS, timeoutMs);

  std::vector<expresso::core::io::Event> result;
  if (count < 0) {
    return result;
  }

  result.reserve(count);
  for (int i = 0; i < count; i++) {
    expresso::enums::NetworkEventType type =
        events[i].events & EPOLLOUT ? expresso::enums::NetworkEventType::WRITE
                                    : expresso::enums::NetworkEventType::READ;
    result.push_back({events[i].data.fd, type});
  }

  return result;
}

#endif
