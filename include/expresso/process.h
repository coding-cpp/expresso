#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <string>

class Process {
private:
  std::map<std::string, std::string> env;

public:
  Process(std::string envFile);
  ~Process();

  std::string getEnv(std::string key);
};