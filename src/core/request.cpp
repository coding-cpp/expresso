#include <expresso/core/request.h>

expresso::core::Request::Request() {
  this->xhr = false;
  this->res = nullptr;

  return;
}

expresso::core::Request::~Request() { return; }

void expresso::core::Request::print() {
  utils::print::info("Request: ");

  utils::print::info("  host: " + this->host);
  utils::print::info("  hostname: " + this->hostname);
  utils::print::info("  method: " + this->method);
  utils::print::info("  path: " + this->path);

  utils::print::info("  body: ");
  for (const std::pair<std::string, std::string> &body : this->body) {
    utils::print::info("    " + body.first + ": " + body.second);
  }

  utils::print::info("  cookies: ");
  for (const std::pair<std::string, std::string> &cookie : this->cookies) {
    utils::print::info("    " + cookie.first + ": " + cookie.second);
  }

  utils::print::info("  headers: ");
  for (const std::pair<std::string, std::string> &header : this->headers) {
    utils::print::info("    " + header.first + ": " + header.second);
  }

  utils::print::info("  params: ");
  for (const std::pair<std::string, std::string> &param : this->params) {
    utils::print::info("    " + param.first + ": " + param.second);
  }

  utils::print::info("  queries: ");
  for (const std::pair<std::string, std::string> &query : this->queries) {
    utils::print::info("    " + query.first + ": " + query.second);
  }

  return;
}