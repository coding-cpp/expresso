#pragma once

#include <map>
#include <string>

typedef struct {
  std::map<std::string, std::string> params;
  std::map<std::string, std::string> ids;
} Body;

typedef struct {
  std::string method;
  std::string path;

  std::map<std::string, std::string> headers;

  Body body;
} Request;