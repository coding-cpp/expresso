#pragma once

#include <set>
#include <string>
#include <unordered_map>

namespace expresso {

namespace enums {

enum class method { GET, POST, PUT, PATCH, DELETE, OPTIONS, HEAD };

extern const std::set<std::string> methods;

} // namespace enums

} // namespace expresso

bool operator==(const expresso::enums::method &m, const std::string &str);
bool operator!=(const expresso::enums::method &m, const std::string &str);

std::ostream &operator<<(std::ostream &os, const expresso::enums::method &m);

std::string operator+(const expresso::enums::method &m, const std::string &str);
std::string operator+(const std::string &str, const expresso::enums::method &m);
