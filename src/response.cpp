#include <expresso/response.h>

Response::Response(int clientSocket) {
  this->socket = clientSocket;

  return;
}

Response::~Response() { return; }

Response Response::status(int statusCode) {
  this->statusCode = statusCode;

  return *this;
}

void Response::json(std::string response) {
  std::string header =
      "HTTP/1.1 " + std::to_string(this->statusCode) + " OK\r\n";
  header += "Content-Type: text/plain\r\n";
  header += "Content-Length: " + std::to_string(response.length()) + "\r\n";
  header += "\r\n";

  send(this->socket, header.c_str(), header.length(), 0);
  send(this->socket, response.c_str(), response.length(), 0);

  return;
}