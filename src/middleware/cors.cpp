#include <expresso/constants/cors.h>
#include <expresso/middleware/cors.h>

expresso::middleware::Cors::Cors()
  : credentials(false), allowAllOrigins(false), allowedHeaders("") {
  for (std::string _header : expresso::constants::cors::headers) {
    this->headers.insert(_header);
  }

  this->allowMethod(mochios::enums::method::GET);
  this->allowMethod(mochios::enums::method::POST);
  this->allowMethod(mochios::enums::method::OPTIONS);
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
      mochios::enums::methods.find(method);
  if (methodIter == mochios::enums::methods.end()) {
    logger::error(
        "Invalid CORS method: " + method,
        "void expresso::middleware::Cors::allowMethod(std::string method)");
    return;
  }

  this->methods.insert(static_cast<mochios::enums::method>(
    std::distance(mochios::enums::methods.begin(), methodIter) - 1));
  return;
}

void expresso::middleware::Cors::allowMethod(mochios::enums::method method) {
  this->methods.insert(method);
  return;
}

void expresso::middleware::Cors::allowHeader(std::string header) {
  this->headers.insert(brewtils::string::lower(header));
  this->allowedHeaders = brewtils::string::join(this->headers, ", ");
  return;
}

void expresso::middleware::Cors::allowCredentials(bool credentials) {
  this->credentials = credentials;
  return;
}

bool expresso::middleware::Cors::use(expresso::messages::Request& req,
                                     expresso::messages::Response& res) {
  if (this->allowAllOrigins) {
    return true;
  }

  std::string requestOrigin = req.headers["origin"];
  if (requestOrigin == "") {
    res.set("access-control-allow-origin", "null");
    res.status(expresso::enums::STATUS_CODE::FORBIDDEN)
       .send(expresso::constants::cors::forbidden);
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
       .send(expresso::constants::cors::forbidden);
    return false;
  }

  res.set("access-control-allow-credentials",
          this->credentials ? "true" : "false");
  res.set("access-control-allow-headers",
          this->allowedHeaders == "" ? "*" : this->allowedHeaders);
  return true;
}