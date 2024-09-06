#pragma once

#include <expresso/core/request.h>

namespace expresso {

namespace middleware {

class Middleware {
private:
public:
  Middleware();
  virtual ~Middleware() = default;

  virtual bool use(expresso::core::Request &req,
                   expresso::core::Response &res) = 0;
};

} // namespace middleware

} // namespace expresso
