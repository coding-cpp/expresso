#include <expresso/middleware/middleware.h>
#include <expresso/version.h>

namespace expresso {

namespace middleware {

class Version : public Middleware {
private:
  std::string version;

public:
  Version();
  ~Version();

  bool use(expresso::messages::Request &request,
           expresso::messages::Response &response) override;
};

} // namespace middleware

} // namespace expresso