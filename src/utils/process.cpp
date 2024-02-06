#include <expresso/utils/process.h>

expresso::utils::Process::Process() : Process("../.env") { return; }

expresso::utils::Process::Process(std::string envFile) {
  std::fstream file(envFile, std::ios::in);
  if (!file.is_open()) {
    logger::error("Unable to open \".env\" file.");
  }

  std::string line;
  while (std::getline(file, line)) {
    std::string key = line.substr(0, line.find("="));
    std::string value = line.substr(line.find("=") + 1, line.length());
    this->env[key] = value;
  }

  return;
}

expresso::utils::Process::~Process() { return; }

std::string expresso::utils::Process::getEnv(std::string key) {
  if (this->env.find(key) != this->env.end()) {
    return this->env[key];
  } else {
    try {
      std::string value = getenv(key.c_str());
      return value;
    } catch (const std::exception &e) {
      logger::error("Environment variable \"" + key + "\" not found!");
      return "";
    }
  }
}