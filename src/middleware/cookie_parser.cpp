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
  for (std::string &cookie : cookies) {
    std::vector<std::string> cookiePair = utils::split(cookie, "=");
    req.cookies[utils::trim(cookiePair[0])] = utils::trim(cookiePair[1]);
  }

  return true;
}