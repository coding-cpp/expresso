#include <expresso/middleware/middleware.h>
#include <expresso/version.h>

namespace expresso {

namespace middleware {

class Expresso : public Middleware {
private:
  std::string version;

public:
  Expresso();
  ~Expresso();

  bool use(expresso::core::Request &request,
           expresso::core::Response &response) override;
};

} // namespace middleware

} // namespace expresso