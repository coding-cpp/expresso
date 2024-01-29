#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class CookieParser : public Middleware {
private:
public:
  CookieParser();
  ~CookieParser();

  bool use(core::Request &req, core::Response &res) override;
};

} // namespace middleware

} // namespace expresso