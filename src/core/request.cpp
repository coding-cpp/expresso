#include <expresso/core/request.h>

expresso::core::Request::Request() {
  this->xhr = false;
  this->res = nullptr;

  return;
}

expresso::core::Request::~Request() { return; }

void expresso::core::Request::print() {
  logger::info("Request: ");

  logger::info("  host: " + this->host);
  logger::info("  hostname: " + this->hostname);
  logger::info("  method: " + this->method);
  logger::info("  path: " + this->path);

  logger::info("  body: ");
  for (const std::pair<std::string, std::string> &body : this->body) {
    logger::info("    " + body.first + ": " + body.second);
  }

  logger::info("  cookies: ");
  for (const std::pair<std::string, std::string> &cookie : this->cookies) {
    logger::info("    " + cookie.first + ": " + cookie.second);
  }

  logger::info("  headers: ");
  for (const std::pair<std::string, std::string> &header : this->headers) {
    logger::info("    " + header.first + ": " + header.second);
  }

  logger::info("  params: ");
  for (const std::pair<std::string, std::string> &param : this->params) {
    logger::info("    " + param.first + ": " + param.second);
  }

  logger::info("  queries: ");
  for (const std::pair<std::string, std::string> &query : this->queries) {
    logger::info("    " + query.first + ": " + query.second);
  }

  return;
}