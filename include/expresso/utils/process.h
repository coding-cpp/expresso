#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <logger/log.h>

namespace expresso {

namespace utils {

class Process {
private:
  std::map<std::string, std::string> env;

public:
  Process();
  Process(std::string envFile);
  ~Process();

  std::string getEnv(std::string key);
};

} // namespace utils

} // namespace expresso
