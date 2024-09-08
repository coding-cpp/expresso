#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class Cors : public Middleware {
private:
  bool credentials;
  bool allowAllOrigins;

  std::set<std::string> origins;
  std::set<std::string> headers;
  std::set<expresso::enums::method> methods;

public:
  Cors();
  ~Cors();

  void allowOrigin(std::string origin);
  void allowMethod(std::string method);
  void allowMethod(expresso::enums::method method);
  void allowHeader(std::string header);
  void allowCredentials(bool credentials);

  bool use(expresso::core::Request &req,
           expresso::core::Response &res) override;

  static const std::string FORBIDDEN;
  static const std::set<std::string> HEADERS;
};

} // namespace middleware

} // namespace expresso