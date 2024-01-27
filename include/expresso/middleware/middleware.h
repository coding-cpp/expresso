#pragma once

#include <expresso/core/request.h>
#include <expresso/core/response.h>
#include <expresso/utils/common.h>
#include <expresso/utils/print.h>

namespace expresso {

namespace middleware {

class Middleware {
private:
  /* data */

public:
  Middleware();
  ~Middleware();

  virtual bool use(core::Request &req, core::Response &res) = 0;
};

} // namespace middleware

} // namespace expresso