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

std::vector<std::string> expresso::utils::split(std::string &data,
                                                std::string delimiter) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = data.find(delimiter);

  while (end != std::string::npos) {
    result.push_back(data.substr(start, end - start));
    start = end + delimiter.length();
    end = data.find(delimiter, start);
  }

  result.push_back(data.substr(start, end));

  return result;
}

std::string expresso::utils::trim(std::string &data) {
  size_t start = data.find_first_not_of(" \n\r\t");
  size_t end = data.find_last_not_of(" \n\r\t");

  if (start == std::string::npos || end == std::string::npos) {
    return "";
  }

  return data.substr(start, end - start + 1);
}