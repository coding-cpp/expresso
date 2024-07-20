#include <expresso/middleware/cors.h>

namespace expresso::middleware {

const std::set<std::string> Cors::METHODS = {"GET", "POST", "PUT", "DELETE",
                                             "OPTIONS"};

const std::set<std::string> Cors::HEADERS = {
    "Accept",
    "Access-Control-Allow-Credentials",
    "Access-Control-Allow-Headers",
    "Access-Control-Allow-Methods",
    "Access-Control-Allow-Origin",
    "Access-Control-Expose-Headers",
    "Access-Control-Max-Age",
    "Authorization",
    "Content-Type",
    "Origin",
    "User-Agent",
    "X-Requested-With",
};

const std::string Cors::FORBIDDEN = "Forbidden";

} // namespace expresso::middleware

expresso::middleware::Cors::Cors()
    : credentials(false), allowAllOrigins(false) {
  for (std::string _header : this->HEADERS) {
    this->headers.insert(_header);
  }

  this->allowMethod("GET");
  this->allowMethod("POST");
  this->allowMethod("OPTIONS");

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
  if (this->METHODS.find(method) == this->METHODS.end()) {
    logger::warning("Invalid CORS method: " + method);
  }

  this->methods.insert(method);

  return;
}

void expresso::middleware::Cors::allowHeader(std::string header) {
  this->headers.insert(header);

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

  std::string requestOrigin = req.headers["Origin"];

  if (requestOrigin == "") {
    res.set("Access-Control-Allow-Origin", "null");
    res.status(expresso::core::STATUS_CODE::FORBIDDEN)
        .send(expresso::middleware::Cors::FORBIDDEN);
    return false;
  }

  bool isOriginPresent = false;

  for (std::string origin : this->origins) {
    if (std::regex_match(requestOrigin, std::regex(origin))) {
      res.set("Access-Control-Allow-Origin", origin.substr(1, origin.size()));
      isOriginPresent = true;
      break;
    }
  }

  if (!isOriginPresent) {
    res.set("Access-Control-Allow-Origin", "null");
    res.status(expresso::core::STATUS_CODE::FORBIDDEN)
        .send(expresso::middleware::Cors::FORBIDDEN);
    return false;
  }

  res.set("Access-Control-Allow-Credentials",
          this->credentials ? "true" : "false");
  res.set("Access-Control-Allow-Methods",
          brewtils::string::join(this->methods, ", "));
  res.set("Access-Control-Allow-Headers",
          brewtils::string::join(this->headers, ", "));

  return true;
}