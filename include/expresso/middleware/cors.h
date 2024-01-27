#pragma once

#include <regex>

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class Cors : public Middleware {
private:
  bool credentials;

  std::set<std::string> origins;
  std::set<std::string> methods;
  std::set<std::string> headers;

public:
  Cors();
  ~Cors();

  static std::set<std::string> METHODS;
  static std::set<std::string> HEADERS;

  void allowOrigin(std::string origin);
  void allowMethod(std::string method);
  void allowHeader(std::string header);
  void allowCredentials(bool credentials);

  bool use(core::Request &req, core::Response &res) override;
};

} // namespace middleware

} // namespace expresso