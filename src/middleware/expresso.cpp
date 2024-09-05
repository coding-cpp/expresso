#include <expresso/middleware/expresso.h>

expresso::middleware::Expresso::Expresso() {
  this->version = "v" + std::to_string(EXPRESSO_VERSION_MAJOR) + "." +
                  std::to_string(EXPRESSO_VERSION_MINOR) + "." +
                  std::to_string(EXPRESSO_VERSION_PATCH);
  return;
}

expresso::middleware::Expresso::~Expresso() { return; }

bool expresso::middleware::Expresso::use(expresso::core::Request &request,
                                         expresso::core::Response &response) {
  response.set("expresso", this->version);
  response.set("x-powered-by", "Expresso/" + this->version);
  return true;
}