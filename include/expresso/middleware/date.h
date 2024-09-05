#include <brewtils/date.h>

#include <expresso/middleware/middleware.h>

namespace expresso {

namespace middleware {

class Date : public Middleware {

public:
  Date();
  ~Date();

  bool use(expresso::core::Request &req,
           expresso::core::Response &res) override;
};

} // namespace middleware

} // namespace expresso