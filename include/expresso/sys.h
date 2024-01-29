#pragma once

#include <sys/socket.h>

namespace expresso {

namespace sys {

inline size_t send(int sockfd, const void *buf, size_t len, int flags) {
  return ::send(sockfd, buf, len, flags);
}

inline int socket(int domain, int type, int protocol) noexcept(true) {
  return ::socket(domain, type, protocol);
}

inline int listen(int fd, int n) noexcept(true) { return ::listen(fd, n); }

} // namespace sys

} // namespace expresso
