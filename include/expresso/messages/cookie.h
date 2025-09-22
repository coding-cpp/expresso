#pragma once

#include <mochios/messages/cookie.h>

namespace expresso {

namespace messages {

class Cookie : public mochios::messages::Cookie {
public:
  Cookie();
  Cookie(const std::string& data);
  ~Cookie();
};

} // namespace messages

} // namespace expresso