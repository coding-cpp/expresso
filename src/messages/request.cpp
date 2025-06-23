#include <expresso/messages/request.h>

expresso::messages::Request::Request(const std::string &path)
    : xhr(false), res(nullptr), contentLength(0),
      mochios::messages::Request(path) {
  return;
}

expresso::messages::Request::~Request() { return; }

const void expresso::messages::Request::print() const {
  logger::info("Request: ");

  logger::info("  host: " + this->host);
  logger::info("  hostname: " + this->hostname);
  logger::info("  method: " + this->method);
  logger::info("  path: " + this->path);

  logger::info("  body: ");
  logger::info(this->body.dumps(2, 2));

  logger::info("  cookies: ");
  for (expresso::messages::Cookie *cookie : this->cookies) {
    logger::info("    " + cookie->serialize());
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