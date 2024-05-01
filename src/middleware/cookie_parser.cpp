#include <expresso/middleware/cookie_parser.h>

expresso::middleware::CookieParser::CookieParser() { return; }

expresso::middleware::CookieParser::~CookieParser() { return; }

bool expresso::middleware::CookieParser::use(expresso::core::Request &req,
                                             expresso::core::Response &res) {
  std::string cookieString = req.headers["Cookie"];
  if (cookieString.empty()) {
    return true;
  }

  std::vector<std::string> cookies = utils::split(cookieString, ";");
  std::vector<std::string> cookiePair;
  for (std::string &cookieStr : cookies) {
    expresso::core::Cookie *cookie = new expresso::core::Cookie();
    cookiePair = utils::split(cookieStr, "=");
    cookie->name = utils::trim(cookiePair[0]);
    cookie->value = utils::trim(cookiePair[1]);
    cookie->value = utils::urlDecode(cookie->value);
    req.cookies.push_back(cookie);
  }

  return true;
}