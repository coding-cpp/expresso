#include <expresso/utils/common.h>

std::string expresso::utils::join(std::set<std::string> &set,
                                  std::string delimiter) {
  std::string result = "";

  if (set.size() == 0) {
    return result;
  }

  result += *set.begin();
  for (std::string it : set) {
    if (it == *set.begin()) {
      continue;
    }

    result += delimiter + it;
  }

  return result;
}