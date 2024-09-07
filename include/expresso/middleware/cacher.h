#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class Cacher : public Middleware {
private:
  bool isPrivate;
  uint maxAge;

  std::string serialized;

public:
  Cacher(uint maxAge = 3600, bool isPrivate = false);
  ~Cacher();

  bool use(expresso::core::Request &req,
           expresso::core::Response &res) override;
};

} // namespace middleware

} // namespace expresso