#ifdef __APPLE__

#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

#include <logger/log.h>

#include <expresso/core/io/poller.h>

expresso::core::io::Poller::Poller() noexcept(false) {
  this->pollerFd = kqueue();
  if (this->pollerFd < 0) {
    logger::error("Unable to create kqueue instance!",
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
  struct kevent event;
  int filter = type == expresso::enums::NetworkEventType::READ ? EVFILT_READ
                                                               : EVFILT_WRITE;
  EV_SET(&event, fd, filter, EV_ADD, 0, 0, nullptr);
  if (kevent(this->pollerFd, &event, 1, nullptr, 0, nullptr) < 0) {
    logger::error("Unable to watch fd " + std::to_string(fd) + "!",
                  "void expresso::core::io::Poller::add(int fd, "
                  "expresso::enums::NetworkEventType type)");
  }

  return;
}

void expresso::core::io::Poller::remove(int fd) {
  // A connection is only ever watched for one filter at a time; deleting
  // both tolerates removal from any state, and a missing filter is harmless.
  struct kevent events[2];
  EV_SET(&events[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  EV_SET(&events[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
  kevent(this->pollerFd, &events[0], 1, nullptr, 0, nullptr);
  kevent(this->pollerFd, &events[1], 1, nullptr, 0, nullptr);

  return;
}

std::vector<expresso::core::io::Event>
expresso::core::io::Poller::wait(int timeoutMs) {
  struct timespec timeout;
  struct timespec* timeoutPtr = nullptr;
  if (timeoutMs >= 0) {
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_nsec = (timeoutMs % 1000) * 1000000L;
    timeoutPtr = &timeout;
  }

  struct kevent events[Poller::MAX_EVENTS];
  int count = kevent(this->pollerFd, nullptr, 0, events, Poller::MAX_EVENTS,
                     timeoutPtr);

  std::vector<expresso::core::io::Event> result;
  if (count < 0) {
    return result;
  }

  result.reserve(count);
  for (int i = 0; i < count; i++) {
    expresso::enums::NetworkEventType type =
        events[i].filter == EVFILT_WRITE
            ? expresso::enums::NetworkEventType::WRITE
            : expresso::enums::NetworkEventType::READ;
    result.push_back({static_cast<int>(events[i].ident), type});
  }

  return result;
}

#endif
