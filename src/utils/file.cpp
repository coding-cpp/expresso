#include <expresso/utils/file.h>

bool expresso::utils::isFile(std::string path) {
  return std::filesystem::exists(path) &&
         std::filesystem::is_regular_file(path);
}

std::string expresso::utils::joinPath(std::string dirname,
                                      std::string filename) {
  if (dirname.back() == '/') {
    dirname.pop_back();
  }

  if (filename.substr(0, 2) == "./") {
    filename = filename.substr(2);
  }

  return dirname + "/" + filename;
};

std::string expresso::utils::getAvailableFile(std::string path) {
  if (utils::isFile(path)) {
    return path;
  }

  std::string tempPath = utils::joinPath(path, "index.htm");
  if (utils::isFile(tempPath)) {
    return tempPath;
  }

  tempPath.push_back('l');
  if (utils::isFile(tempPath)) {
    return tempPath;
  }

  return "";
}