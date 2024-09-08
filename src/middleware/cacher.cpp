#include <expresso/middleware/cacher.h>

expresso::middleware::Cacher::Cacher(uint32_t maxAge, bool isPrivate)
    : isPrivate(isPrivate), maxAge(maxAge) {
  this->serialized = "max-age=" + std::to_string(maxAge);
  if (isPrivate) {
    this->serialized += ", private";
  } else {
    this->serialized += ", public";
  }
}

expresso::middleware::Cacher::~Cacher() { return; }

bool expresso::middleware::Cacher::use(expresso::core::Request &req,
                                       expresso::core::Response &res) {
  res.set("cache-control", this->serialized);
  return true;
}