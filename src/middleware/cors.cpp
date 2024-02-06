#include <expresso/middleware/cors.h>

expresso::middleware::Cors::Cors() {
  this->credentials = false;
  this->allowAllOrigins = false;

  for (std::string _header : this->HEADERS) {
    this->headers.insert(_header);
  }

  this->allowMethod("GET");
  this->allowMethod("POST");
  this->allowMethod("OPTIONS");

  return;
}

expresso::middleware::Cors::~Cors() { return; }

std::set<std::string> expresso::middleware::Cors::METHODS = {
    "GET", "POST", "PUT", "DELETE", "OPTIONS"};

std::set<std::string> expresso::middleware::Cors::HEADERS = {
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

bool expresso::middleware::Cors::use(core::Request &req, core::Response &res) {
  if (this->allowAllOrigins) {
    return true;
  }

  std::string requestOrigin = req.headers["Origin"];

  if (requestOrigin == "") {
    res.set("Access-Control-Allow-Origin", "null");
    res.status(core::StatusCode::FORBIDDEN).send("Forbidden");
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
    res.status(core::StatusCode::FORBIDDEN).send("Forbidden");
    return false;
  }

  res.set("Access-Control-Allow-Credentials",
          this->credentials ? "true" : "false");
  res.set("Access-Control-Allow-Methods", utils::join(this->methods, ", "));
  res.set("Access-Control-Allow-Headers", utils::join(this->headers, ", "));

  return true;
}