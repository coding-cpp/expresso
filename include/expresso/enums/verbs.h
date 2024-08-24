#pragma once

#include <set>
#include <string>

namespace expresso {

namespace enums {

static const std::set<std::string> VERBS = {"GET",    "POST",    "PUT", "PATCH",
                                            "DELETE", "OPTIONS", "HEAD"};

} // namespace enums

} // namespace expresso