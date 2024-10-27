#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class CookieParser : public Middleware {
private:
public:
  CookieParser();
  ~CookieParser();

  bool use(expresso::messages::Request &req,
           expresso::messages::Response &res) override;
};

} // namespace middleware

} // namespace expresso