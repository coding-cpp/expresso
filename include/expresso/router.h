#pragma once

#include <iostream>
#include <map>

#include <netinet/in.h>
#include <sys/socket.h>

class Router {
private:
  std::string basePath;

  std::map<std::string, void (*)(int clientSocket)> getMap;
  std::map<std::string, void (*)(int clientSocket)> postMap;
  std::map<std::string, void (*)(int clientSocket)> putMap;
  std::map<std::string, void (*)(int clientSocket)> patchMap;
  std::map<std::string, void (*)(int clientSocket)> delMap;
  std::map<std::string, void (*)(int clientSocket)> optionsMap;

public:
  Router();
  ~Router();

  void setBasePath(std::string basePath);
  std::string getBasePath();

  void get(std::string path, void (*handler)(int clientSocket));
  void post(std::string path, void (*handler)(int clientSocket));
  void put(std::string path, void (*handler)(int clientSocket));
  void patch(std::string path, void (*handler)(int clientSocket));
  void del(std::string path, void (*handler)(int clientSocket));
  void options(std::string path, void (*handler)(int clientSocket));

  void handleRequest(int clientSocket, std::string method, std::string path);
};