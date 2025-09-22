#pragma once

#include <expresso/messages/request.h>

namespace expresso {

namespace middleware {

class Middleware {
private:
public:
  Middleware();
  virtual ~Middleware() = default;

  virtual bool use(expresso::messages::Request& req,
                   expresso::messages::Response& res) = 0;
};

} // namespace middleware

} // namespace expresso