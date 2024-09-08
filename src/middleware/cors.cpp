#include <expresso/middleware/cors.h>

namespace expresso::middleware {

const std::set<std::string> Cors::HEADERS = {
    "accept",
    "access-control-allow-credentials",
    "access-control-allow-headers",
    "access-control-allow-methods",
    "access-control-allow-origin",
    "access-control-expose-headers",
    "access-control-max-age",
    "authorization",
    "content-type",
    "origin",
    "user-agent",
    "x-requested-with",
};

const std::string Cors::FORBIDDEN = "Forbidden";

} // namespace expresso::middleware

expresso::middleware::Cors::Cors()
    : credentials(false), allowAllOrigins(false) {
  for (std::string _header : this->HEADERS) {
    this->headers.insert(_header);
  }

  this->allowMethod(expresso::enums::method::GET);
  this->allowMethod(expresso::enums::method::POST);
  this->allowMethod(expresso::enums::method::OPTIONS);

  return;
}

expresso::middleware::Cors::~Cors() { return; }

void expresso::middleware::Cors::allowOrigin(std::string origin) {
  if (origin[0] != '.') {
    origin = "." + origin;
  }
  this->origins.insert(origin);

  if (origin == ".*") {
    this->allowAllOrigins = true;
  }

  return;
}

void expresso::middleware::Cors::allowMethod(std::string method) {
  std::set<std::string>::const_iterator methodIter =
      expresso::enums::methods.find(method);
  if (methodIter == expresso::enums::methods.end()) {
    logger::warning("Invalid CORS method: " + method);
  }

  this->methods.insert(static_cast<expresso::enums::method>(
      std::distance(expresso::enums::methods.begin(), methodIter) - 1));

  return;
}

void expresso::middleware::Cors::allowMethod(expresso::enums::method method) {
  this->methods.insert(method);

  return;
}

void expresso::middleware::Cors::allowHeader(std::string header) {
  this->headers.insert(brewtils::string::lower(header));

  return;
}

void expresso::middleware::Cors::allowCredentials(bool credentials) {
  this->credentials = credentials;

  return;
}

bool expresso::middleware::Cors::use(expresso::core::Request &req,
                                     expresso::core::Response &res) {
  if (this->allowAllOrigins) {
    return true;
  }

  std::string requestOrigin = req.headers["origin"];

  if (requestOrigin == "") {
    res.set("access-control-allow-origin", "null");
    res.status(expresso::enums::STATUS_CODE::FORBIDDEN)
        .send(expresso::middleware::Cors::FORBIDDEN);
    return false;
  }

  bool isOriginPresent = false;

  for (std::string origin : this->origins) {
    if (std::regex_match(requestOrigin, std::regex(origin))) {
      res.set("access-control-allow-origin", origin.substr(1, origin.size()));
      isOriginPresent = true;
      break;
    }
  }

  if (!isOriginPresent) {
    res.set("access-control-allow-origin", "null");
    res.status(expresso::enums::STATUS_CODE::FORBIDDEN)
        .send(expresso::middleware::Cors::FORBIDDEN);
    return false;
  }

  res.set("access-control-allow-credentials",
          this->credentials ? "true" : "false");
  res.set("access-control-allow-headers",
          brewtils::string::join(this->headers, ", "));

  return true;
}