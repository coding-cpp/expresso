#include <expresso/enums/method.h>

const std::set<std::string> expresso::enums::methods = {
    "GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS", "HEAD"};

static const std::unordered_map<expresso::enums::method, std::string>
    methodToString{{expresso::enums::method::GET, "GET"},
                   {expresso::enums::method::POST, "POST"},
                   {expresso::enums::method::PUT, "PUT"},
                   {expresso::enums::method::PATCH, "PATCH"},
                   {expresso::enums::method::DELETE, "DELETE"},
                   {expresso::enums::method::OPTIONS, "OPTIONS"},
                   {expresso::enums::method::HEAD, "HEAD"}};

bool operator==(const expresso::enums::method &m, const std::string &str) {
  return methodToString.at(m) == str;
}

bool operator!=(const expresso::enums::method &m, const std::string &str) {
  return !(m == str);
}

std::ostream &operator<<(std::ostream &os, const expresso::enums::method &m) {
  os << methodToString.at(m);
  return os;
}

std::string operator+(const expresso::enums::method &m,
                      const std::string &str) {
  return methodToString.at(m) + str;
}

std::string operator+(const std::string &str,
                      const expresso::enums::method &m) {
  return str + methodToString.at(m);
}
