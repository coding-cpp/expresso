#pragma once

#include <iostream>
#include <stdexcept>

namespace expresso {

namespace utils {

namespace print {

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;36m"
#define RESET "\033[0m"

void success(const std::string message, bool newLine = true);

void info(const std::string message, bool newLine = true);

void warning(const std::string message, bool newLine = true);

void error(const std::string message,
           const std::string runtimeErrorMessage = "", bool newLine = true);

} // namespace print

} // namespace utils

} // namespace expresso