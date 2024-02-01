#pragma once

#include <expresso/middleware/middleware.h>
#include <expresso/utils/file.h>
#include <expresso/utils/sterlize.h>

namespace expresso {

namespace middleware {

class StaticServe : public Middleware {
private:
  std::string dirname;

public:
  StaticServe(std::string dirname);
  ~StaticServe();

  bool use(core::Request &req, core::Response &res) override;
};

} // namespace middleware

} // namespace expresso