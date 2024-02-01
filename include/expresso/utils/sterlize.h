#pragma once

#include <iostream>
#include <string>

namespace expresso {

namespace utils {

std::string base64Encode(std::string &data);

std::string base64Decode(std::string &data);

std::string urlEncode(std::string &data);

std::string urlDecode(std::string &data);

} // namespace utils

} // namespace expresso
