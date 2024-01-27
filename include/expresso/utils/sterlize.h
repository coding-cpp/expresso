#pragma once

#include <iostream>
#include <string>

namespace expresso {

namespace utils {

std::string base64_encode(std::string &data);

std::string base64_decode(std::string &data);

std::string url_encode(std::string &data);

std::string url_decode(std::string &data);

} // namespace utils

} // namespace expresso
