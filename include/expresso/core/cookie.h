#pragma once

#include <string>

#include <expresso/utils/sterlize.h>

namespace expresso {

namespace core {

class Cookie {
public:
  bool secure;
  bool httpOnly;

  std::string name;
  std::string value;
  std::string domain;
  std::string path;
  std::string expires;
  std::string maxAge;
  std::string sameSite;

  Cookie();
  ~Cookie();

  std::string serialize();
};

} // namespace core

} // namespace expresso