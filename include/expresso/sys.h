#pragma once

// Namespace the <sys/socket.h> file
namespace sys {

#include <sys/socket.h>

inline size_t send_wrapper(int socket, const void *buffer, size_t length,
                           int flags) {
  return send(socket, buffer, length, flags);
}

} // namespace sys
