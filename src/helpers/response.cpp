#include <expresso/helpers/response.h>

std::string expresso::helpers::getAvailableFile(const std::string &path) {
  if (brewtils::os::file::exists(path)) {
    return path;
  }

  std::string tempPath = brewtils::os::joinPath(path, "index.htm");
  if (brewtils::os::file::exists(tempPath)) {
    return tempPath;
  }

  tempPath.push_back('l');
  if (brewtils::os::file::exists(tempPath)) {
    return tempPath;
  }

  return "";
}