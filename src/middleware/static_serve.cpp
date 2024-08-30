#include <expresso/helpers/response.h>
#include <expresso/middleware/static_serve.h>

expresso::middleware::StaticServe::StaticServe(std::string dirname,
                                               bool showListing)
    : dirname(std::move(dirname)), showListing(showListing) {
  return;
}

expresso::middleware::StaticServe::~StaticServe() { return; }

bool expresso::middleware::StaticServe::use(expresso::core::Request &req,
                                            expresso::core::Response &res) {
  if (req.method != "GET") {
    return true;
  }

  std::pair<int64_t, int64_t> range = this->getRange(req.headers["range"]);
  std::string filePath = brewtils::url::decode(req.tempPath);
  std::string availableFile = expresso::helpers::getAvailableFile(
      brewtils::os::joinPath(this->dirname, filePath));
  if (!availableFile.empty()) {
    res.sendFile(availableFile, range.first, range.second);
    return false;
  }

  if (this->showListing && brewtils::os::dir::exists(brewtils::os::joinPath(
                               this->dirname, filePath))) {
    const std::string &tempPath =
        brewtils::os::joinPath(this->dirname, filePath);
    if (!brewtils::os::dir::exists(tempPath)) {
      return true;
    }

    res.send(this->getFolderHTML(req, tempPath));
    res.set("Content-Type", "text/html");
    res.status(expresso::enums::STATUS_CODE::OK);
    return false;
  }

  return true;
}

std::pair<int64_t, int64_t>
expresso::middleware::StaticServe::getRange(const std::string &range) {
  int64_t start = -1;
  int64_t end = -1;
  const std::vector<std::string> parts =
      brewtils::string::split(range, "bytes=");
  if (parts.size() == 2) {
    const std::vector<std::string> rangeParts =
        brewtils::string::split(parts[1], "-");
    if (rangeParts.size() > 0) {
      start = std::stoll(rangeParts[0]);
      if (rangeParts.size() > 1) {
        end = std::stoll(rangeParts[1]);
      }
    }
  }
  return std::make_pair(start, end);
}

std::string
expresso::middleware::StaticServe::getFolderHTML(expresso::core::Request &req,
                                                 const std::string &localPath) {
  std::ostringstream oss;
  const std::string urlPath = req.path;

  oss << "<html><head><title>Index of /" + urlPath +
             "</title></head><body><h1>Index of /" +
             brewtils::url::decode(urlPath) + "</h1><ul>";
  if (!req.tempPath.empty()) {
    oss << "<li><a href=\"/" + urlPath + "/..\">..</a></li>";
  }

  std::vector<std::string> files = brewtils::os::dir::list(localPath);
  for (const std::string &file : files) {
    std::string fileName = brewtils::string::split(file, "/").back();
    oss << "<li><a href=\"/" + brewtils::os::joinPath(urlPath, fileName) +
               "\">" + fileName;
    if (brewtils::os::dir::exists(file)) {
      oss << "/";
    }
    oss << "</a></li>";
  }

  oss << "</ul></body></html>";
  return oss.str();
}