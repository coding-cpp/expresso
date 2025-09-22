#pragma once

#include <brewtils/date.h>

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class Date : public Middleware {

public:
  Date();
  ~Date();

  bool use(expresso::messages::Request &req,
           expresso::messages::Response &res) override;
};

} // namespace middleware

} // namespace expresso