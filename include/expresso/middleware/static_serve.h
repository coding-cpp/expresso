#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class StaticServe : public Middleware {
private:
  bool showListing;

  std::string dirname;

public:
  StaticServe(std::string dirname, bool showListing = false);
  ~StaticServe();

  bool use(core::Request &req, core::Response &res) override;
};

} // namespace middleware

} // namespace expresso