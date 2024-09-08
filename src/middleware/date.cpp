#include <expresso/middleware/date.h>

expresso::middleware::Date::Date() { return; }

expresso::middleware::Date::~Date() { return; }

bool expresso::middleware::Date::use(expresso::core::Request &req,
                                     expresso::core::Response &res) {
  res.set("date", brewtils::date::getCurrentUTC());
  return true;
}