#include <fcntl.h>

#include <logger/log.h>

#include <expresso/core/io/socket.h>

void expresso::core::io::setNonBlocking(int fd) noexcept(false) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    logger::error("Unable to make fd " + std::to_string(fd) + " non-blocking!",
                  "void expresso::core::io::setNonBlocking(int fd)");
  }

  return;
}
