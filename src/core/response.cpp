#include <expresso/core/response.h>

expresso::core::Response::Response(int clientSocket)
    : socket(clientSocket), statusCode(StatusCode::OK) {
  return;
}

expresso::core::Response::~Response() { return; }

expresso::core::Response expresso::core::Response::status(int code) {
  this->statusCode = code;

  return *this;
}

void expresso::core::Response::send(std::string response) {
  response = std::regex_replace(response, std::regex("\n"), "\r\n");
  response += "\r\n";

  std::string header = "HTTP/1.1 " + std::to_string(this->statusCode) + "\r\n";
  header += "Content-Length: " + std::to_string(response.length()) + "\r\n";
  header += "\r\n";

  sys::send(this->socket, header.c_str(), header.length(), 0);
  sys::send(this->socket, response.c_str(), response.length(), 0);

  return;
}