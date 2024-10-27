#pragma once

#include <string>

#include <brewtils/url.h>

namespace expresso {

namespace messages {

class Cookie {
public:
  bool secure;
  bool httpOnly;
  bool partitioned;

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

} // namespace messages

} // namespace expresso