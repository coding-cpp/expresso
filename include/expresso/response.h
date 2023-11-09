#pragma once

#include <string>

#include <netinet/in.h>

#include <expresso/sys.h>

class Response {
private:
  int socket;
  int statusCode;

public:
  Response(int clientSocket);
  ~Response();

  Response status(int statusCode);
  void send(std::string response);
};