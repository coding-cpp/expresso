#include <expresso/process.h>

Process::Process(std::string envFile) {
  std::fstream file(envFile, std::ios::in);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      std::string key = line.substr(0, line.find("="));
      std::string value = line.substr(line.find("=") + 1, line.length());
      this->env[key] = value;
    }
  } else {
    std::cerr << "[ERROR] Unable to open \".env\" file." << std::endl;
  }

  return;
}

Process::~Process() { return; }

std::string Process::getEnv(std::string key) {
  if (this->env.find(key) != this->env.end()) {
    return this->env[key];
  } else {
    try {
      std::string value = getenv(key.c_str());
      return value;
    } catch (const std::exception &e) {
      std::cerr << "[ERROR] Environment variable \"" << key << "\" not found!"
                << std::endl;
      return "";
    }
  }
}