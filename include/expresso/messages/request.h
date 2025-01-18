#pragma once

#include <mochios/messages/request.h>

#include <expresso/messages/response.h>

namespace expresso {

namespace messages {

class Request : public mochios::messages::Request {
private:
public:
  Request(const std::string &path);
  ~Request();

  bool xhr;
  int contentLength;

  std::string host;
  std::string hostname;
  std::string httpVersion;

  // tempPath used internally for referencing. DO NOT USE IT! Use `path`
  // instead.
  std::string tempPath;
  std::vector<expresso::messages::Cookie *> cookies;

  expresso::messages::Response *res;

  void print() override;
};

} // namespace messages

} // namespace expresso
