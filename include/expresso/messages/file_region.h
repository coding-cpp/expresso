#pragma once

#include <sys/types.h>

namespace expresso {

namespace messages {

// A slice of an open file that still has to be streamed to a client. The
// owner of a FileRegion is responsible for closing `fd`.
struct FileRegion {
  int fd = -1;
  off_t offset = 0;
  off_t remaining = 0;

  bool isValid() const { return this->fd >= 0; }
};

} // namespace messages

} // namespace expresso
