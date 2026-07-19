#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

#include <expresso/core/io/connection.h>
#include <expresso/core/io/socket.h>

namespace {
constexpr size_t READ_BUFFER_SIZE = 4096;
}

expresso::core::io::Connection::Connection(int socket) noexcept(false)
  : socket(socket), phase(Phase::READING), bytesWritten(0),
    lastActivity(std::chrono::steady_clock::now()) {
  expresso::core::io::setNonBlocking(socket);

  return;
}

expresso::core::io::Connection::~Connection() {
  if (this->file.isValid()) {
    close(this->file.fd);
  }
  close(this->socket);

  return;
}

int expresso::core::io::Connection::getSocket() const { return this->socket; }

expresso::core::io::Connection::Phase
expresso::core::io::Connection::getPhase() const {
  return this->phase;
}

void expresso::core::io::Connection::setPhase(Phase phase) {
  this->phase = phase;

  return;
}

const std::string& expresso::core::io::Connection::getRequest() const {
  return this->readBuffer;
}

void expresso::core::io::Connection::setResponse(
    std::string&& data, expresso::messages::FileRegion file) {
  this->writeBuffer = std::move(data);
  this->bytesWritten = 0;
  this->file = file;
  this->lastActivity = std::chrono::steady_clock::now();
  this->phase = Phase::WRITING;

  return;
}

bool expresso::core::io::Connection::idleFor(
    std::chrono::seconds duration) const {
  return std::chrono::steady_clock::now() - this->lastActivity > duration;
}

expresso::core::io::Connection::IOResult
expresso::core::io::Connection::read() {
  this->lastActivity = std::chrono::steady_clock::now();
  char buffer[READ_BUFFER_SIZE];

  while (true) {
    ssize_t bytesRead = recv(this->socket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
      this->readBuffer.append(buffer, bytesRead);
      continue;
    }
    if (bytesRead == 0) {
      return IOResult::CLOSED;
    }
    if (errno == EINTR) {
      continue;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return IOResult::PENDING;
    }
    return IOResult::CLOSED;
  }
}

expresso::core::io::Connection::IOResult
expresso::core::io::Connection::write() {
  this->lastActivity = std::chrono::steady_clock::now();
  IOResult result = this->flushBuffer();
  if (result != IOResult::COMPLETE) {
    return result;
  }

  return this->flushFile();
}

expresso::core::io::Connection::IOResult
expresso::core::io::Connection::flushBuffer() {
  while (this->bytesWritten < this->writeBuffer.size()) {
    ssize_t bytesSent =
        send(this->socket, this->writeBuffer.data() + this->bytesWritten,
             this->writeBuffer.size() - this->bytesWritten, 0);
    if (bytesSent < 0) {
      if (errno == EINTR) {
        continue;
      }
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return IOResult::PENDING;
      }
      return IOResult::CLOSED;
    }
    this->bytesWritten += bytesSent;
  }

  return IOResult::COMPLETE;
}

expresso::core::io::Connection::IOResult
expresso::core::io::Connection::flushFile() {
  while (this->file.isValid() && this->file.remaining > 0) {
#ifdef __linux__
    ssize_t bytesSent =
        sendfile(this->socket, this->file.fd, &this->file.offset,
                 static_cast<size_t>(this->file.remaining));
    if (bytesSent < 0) {
      if (errno == EINTR) {
        continue;
      }
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return IOResult::PENDING;
      }
      return IOResult::CLOSED;
    }
    if (bytesSent == 0) {
      // The file is shorter than promised; the response cannot be completed
      return IOResult::CLOSED;
    }
    this->file.remaining -= bytesSent;
#else
    off_t bytesSent = this->file.remaining;
    int status = sendfile(this->file.fd, this->socket, this->file.offset,
                          &bytesSent, nullptr, 0);
    this->file.offset += bytesSent;
    this->file.remaining -= bytesSent;
    if (status < 0) {
      if (errno == EINTR) {
        continue;
      }
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return IOResult::PENDING;
      }
      return IOResult::CLOSED;
    }
    if (bytesSent == 0 && this->file.remaining > 0) {
      // The file is shorter than promised; the response cannot be completed
      return IOResult::CLOSED;
    }
#endif
  }

  return IOResult::COMPLETE;
}
