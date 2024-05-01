#include <expresso/core/cookie.h>

expresso::core::Cookie::Cookie() {
  this->secure = false;
  this->httpOnly = false;
  this->name = "";
  this->value = "";
  this->domain = "";
  this->path = "";
  this->expires = "";
  this->maxAge = "";
  this->sameSite = "";

  return;
}

expresso::core::Cookie::~Cookie() { return; }

std::string expresso::core::Cookie::serialize() {
  std::string cookieString = this->name + "=" + utils::urlEncode(this->value);
  if (!this->domain.empty()) {
    cookieString += "; Domain=" + this->domain;
  }
  if (!this->path.empty()) {
    cookieString += "; Path=" + this->path;
  }
  if (!this->expires.empty()) {
    cookieString += "; Expires=" + this->expires;
  }
  if (!this->maxAge.empty()) {
    cookieString += "; Max-Age=" + this->maxAge;
  }
  if (!this->sameSite.empty()) {
    cookieString += "; SameSite=" + this->sameSite;
  }
  if (this->secure) {
    cookieString += "; Secure";
  }
  if (this->httpOnly) {
    cookieString += "; HttpOnly";
  }

  return cookieString;
}