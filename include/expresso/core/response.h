#pragma once

#include <netinet/in.h>
#include <regex>
#include <string>

#include <expresso/core/status_code.h>
#include <expresso/sys.h>

namespace expresso {

namespace core {

class Response {
private:
  int socket;
  int statusCode;

public:
  Response(int clientSocket);
  ~Response();

  Response status(int code);
  void send(std::string response);
};

} // namespace core

} // namespace expresso