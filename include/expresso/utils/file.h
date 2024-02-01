#pragma once

#include <filesystem>
#include <string>

namespace expresso {

namespace utils {

bool isFile(std::string path);

std::string joinPath(std::string dirname, std::string filename);

std::string getAvailableFile(std::string path);

} // namespace utils

} // namespace expresso