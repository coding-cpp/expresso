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

  utils::print::info("  body: ");
  for (auto const &body : this->body) {
    utils::print::info("    " + body.first + ": " + body.second);
  }

  utils::print::info("  cookies: ");
  for (auto const &cookie : this->cookies) {
    utils::print::info("    " + cookie.first + ": " + cookie.second);
  }

  utils::print::info("  headers: ");
  for (auto const &header : this->headers) {
    utils::print::info("    " + header.first + ": " + header.second);
  }

  utils::print::info("  params: ");
  for (auto const &param : this->params) {
    utils::print::info("    " + param.first + ": " + param.second);
  }

  utils::print::info("  queries: ");
  for (auto const &query : this->queries) {
    utils::print::info("    " + query.first + ": " + query.second);
  }

  return;
}