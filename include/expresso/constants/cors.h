#pragma once

#include <set>
#include <string>

namespace expresso {

namespace constants {

namespace cors {

const std::set<std::string> headers = {"accept",
                                       "access-control-allow-credentials",
                                       "access-control-allow-headers",
                                       "access-control-allow-methods",
                                       "access-control-allow-origin",
                                       "access-control-expose-headers",
                                       "access-control-max-age",
                                       "authorization",
                                       "content-type",
                                       "origin",
                                       "user-agent",
                                       "x-requested-with"};

const std::string forbidden = "Forbidden";

} // namespace cors

} // namespace constants

} // namespace expresso