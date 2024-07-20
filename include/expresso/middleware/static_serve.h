#pragma once

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class StaticServe : public Middleware {
private:
  bool showListing;

  std::string dirname;

  std::string getFolderHTML(expresso::core::Request &req,
                            const std::string &localPath);

public:
  StaticServe(std::string dirname, bool showListing = false);
  ~StaticServe();

  bool use(expresso::core::Request &req,
           expresso::core::Response &res) override;
};

} // namespace middleware

} // namespace expresso