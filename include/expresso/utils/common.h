#pragma once

#include <set>
#include <string>
#include <vector>

namespace expresso {

namespace utils {

std::string join(std::set<std::string> &set, std::string delimiter);

std::vector<std::string> split(std::string &data, std::string delimiter);

std::string trim(std::string &data);

} // namespace utils

} // namespace expresso