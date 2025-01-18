#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class Cors : public Middleware {
private:
  bool credentials;
  bool allowAllOrigins;

  std::string allowedHeaders;

  std::set<std::string> origins;
  std::set<std::string> headers;
  std::set<mochios::enums::method> methods;

public:
  Cors();
  ~Cors();

  void allowOrigin(std::string origin);
  void allowMethod(std::string method);
  void allowMethod(mochios::enums::method method);
  void allowHeader(std::string header);
  void allowCredentials(bool credentials);

  bool use(expresso::messages::Request &req,
           expresso::messages::Response &res) override;
};

} // namespace middleware

} // namespace expresso