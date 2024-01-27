#pragma once

#include <map>
#include <string>

#include <expresso/core/response.h>
#include <expresso/utils/sterlize.h>

namespace expresso {

namespace core {

class Request {
private:
public:
  Request();
  ~Request();

  bool xhr;
  int contentLength;

  std::string host;
  std::string hostname;
  std::string method;
  std::string path;

  std::map<std::string, std::string> body;
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> params;
  std::map<std::string, std::string> queries;

  expresso::core::Response *res;
};

} // namespace core

} // namespace expresso
