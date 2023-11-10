#pragma once

#include <cstring>
#include <iostream>
#include <netdb.h>
#include <unistd.h>

#include <netinet/in.h>

#include <expresso/sterlize.h>
#include <expresso/sys.h>

class Email {
private:
  int port;
  int socket;

  std::string server;
  std::string signature;

  std::string username;
  std::string password;
  std::string encodedUsername;
  std::string encodedPassword;

  struct addrinfo hints, *result;
  struct sockaddr_in *address;

public:
  Email(std::string server, int port);
  ~Email();

  void login(std::string username, std::string password);
  bool send(std::string recipient, std::string subject, std::string message);
  void setSignature(std::string signature);
};