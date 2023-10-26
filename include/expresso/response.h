#pragma once

#include <string>

#include <netinet/in.h>
#include <sys/socket.h>

class Response {
private:
  int socket;
  int statusCode;

public:
  Response(int clientSocket);
  ~Response();

  Response status(int statusCode);
  void json(std::string response);
};