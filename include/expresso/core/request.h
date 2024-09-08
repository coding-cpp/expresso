#pragma once

#include <expresso/core/response.h>

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
  expresso::enums::method method;
  std::string path;
  std::string httpVersion;

  // tempPath used internally for referencing. DO NOT USE IT! Use `path`
  // instead.
  std::string tempPath;

  json::object body;
  std::vector<Cookie *> cookies;
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> params;
  std::map<std::string, std::string> queries;

  expresso::core::Response *res;

  void print();
};

} // namespace core

} // namespace expresso
