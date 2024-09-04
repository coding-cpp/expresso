#include <expresso/middleware/cookie_parser.h>

expresso::middleware::CookieParser::CookieParser() { return; }

expresso::middleware::CookieParser::~CookieParser() { return; }

bool expresso::middleware::CookieParser::use(expresso::core::Request &req,
                                             expresso::core::Response &res) {
  std::string cookieString = req.headers["cookie"];
  if (cookieString.empty()) {
    return true;
  }

  std::vector<std::string> cookies = brewtils::string::split(cookieString, ";");
  std::vector<std::string> cookiePair;
  for (std::string &cookieStr : cookies) {
    expresso::core::Cookie *cookie = new expresso::core::Cookie();
    cookiePair = brewtils::string::split(cookieStr, "=");
    cookie->name = brewtils::string::trim(cookiePair[0]);
    cookie->value = brewtils::string::trim(cookiePair[1]);
    cookie->value = brewtils::url::decode(cookie->value);
    req.cookies.push_back(cookie);
  }

  return true;
}