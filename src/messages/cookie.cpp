#include <expresso/messages/cookie.h>

expresso::messages::Cookie::Cookie() : mochios::messages::Cookie() { return; }

expresso::messages::Cookie::Cookie(const std::string& data)
  : mochios::messages::Cookie(data) {
  return;
}

expresso::messages::Cookie::~Cookie() { return; }