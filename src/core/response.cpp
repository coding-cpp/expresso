#include <expresso/core/response.h>

expresso::core::Response::Response(int clientSocket)
    : socket(clientSocket), statusCode(StatusCode::OK) {
  return;
}

expresso::core::Response::~Response() {
  std::string header = "HTTP/1.1 " + std::to_string(this->statusCode) + "\r\n";
  this->set("Content-Length", std::to_string(this->message.length()));
  for (std::pair<const std::string, std::string> it : this->headers) {
    header += it.first + ": " + it.second + "\r\n";
  }
  header += "\r\n";

  sys::send(this->socket, header.c_str(), header.length(), 0);
  sys::send(this->socket, this->message.c_str(), this->message.length(), 0);

  return;
}

void expresso::core::Response::set(std::string headerName,
                                   std::string headerValue) {
  this->headers[headerName] = headerValue;

  return;
}

std::string expresso::core::Response::get(std::string headerName) {
  if (this->headers.find(headerName) != this->headers.end()) {
    return this->headers[headerName];
  } else {
    return "";
  }
}

expresso::core::Response expresso::core::Response::status(int code) {
  this->statusCode = code;

  return *this;
}

void expresso::core::Response::send(std::string response) {
  this->message = std::regex_replace(response, std::regex("\n"), "\r\n");
  this->message += "\r\n";

  return;
}

void expresso::core::Response::print() {
  utils::print::info("Response: ");

  utils::print::info("  statusCode: " + std::to_string(this->statusCode));

  utils::print::info("  headers: ");
  for (const std::pair<const std::string, std::string> &header :
       this->headers) {
    utils::print::info("    " + header.first + ": " + header.second);
  }

  utils::print::info("  message: " + this->message);

  return;
}