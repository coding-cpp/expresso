#include <expresso/middleware/version.h>

expresso::middleware::Version::Version() {
  this->version = "v" + std::to_string(EXPRESSO_VERSION_MAJOR) + "." +
                  std::to_string(EXPRESSO_VERSION_MINOR) + "." +
                  std::to_string(EXPRESSO_VERSION_PATCH);
  return;
}

expresso::middleware::Version::~Version() { return; }

bool expresso::middleware::Version::use(expresso::core::Request &request,
                                        expresso::core::Response &response) {
  response.set("expresso", this->version);
  response.set("x-powered-by", "Expresso/" + this->version);
  return true;
}