#include <expresso/core/request.h>

expresso::core::Request::Request() {
  this->xhr = false;
  this->res = nullptr;

  return;
}

expresso::core::Request::~Request() { return; }