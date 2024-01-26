#include <expresso/utils/print.h>

namespace expresso::utils::print {

void success(std::string message, bool newLine) {
  std::cout << GREEN << "[SUCCESS]: " << RESET << message;
  if (newLine) {
    std::cout << "\n";
  }
}

void info(std::string message, bool newLine) {
  std::cout << BLUE << "[INFO]   : " << RESET << message;
  if (newLine) {
    std::cout << "\n";
  }
}

void warning(std::string message, bool newLine) {
  std::cout << YELLOW << "[WARNING]: " << RESET << message;
  if (newLine) {
    std::cout << "\n";
  }
}

void error(std::string message, std::string runtimeErrorMessage, bool newLine) {
  std::cerr << RED << "[ERROR]  : " << RESET << message;
  if (runtimeErrorMessage != "") {
    std::cerr << "\n";
    throw std::runtime_error(runtimeErrorMessage);
  }
  if (newLine) {
    std::cerr << "\n";
  }
}

} // namespace expresso::utils::print
