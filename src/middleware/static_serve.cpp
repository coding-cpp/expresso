#include <expresso/middleware/static_serve.h>

expresso::middleware::StaticServe::StaticServe(std::string dirname,
                                               bool showListing)
    : dirname(dirname), showListing(showListing) {
  return;
}

expresso::middleware::StaticServe::~StaticServe() { return; }

bool expresso::middleware::StaticServe::use(expresso::core::Request &req,
                                            expresso::core::Response &res) {
  if (req.method != "GET") {
    return true;
  }

  std::string filePath = brewtils::url::decode(req.tempPath);
  std::string availableFile = expresso::core::Response::getAvailableFile(
      brewtils::os::joinPath(this->dirname, filePath));
  if (!availableFile.empty()) {
    res.sendFile(availableFile);
    return false;
  }

  if (this->showListing && brewtils::os::dir::exists(brewtils::os::joinPath(
                               this->dirname, req.tempPath))) {
    const std::string &tempPath =
        brewtils::os::joinPath(this->dirname, req.tempPath);
    if (!brewtils::os::dir::exists(tempPath)) {
      return true;
    }

    res.send(this->getFolderHTML(req, tempPath));
    res.set("Content-Type", "text/html");
    res.status(expresso::core::STATUS_CODE::OK);
    return false;
  }

  return true;
}

std::string
expresso::middleware::StaticServe::getFolderHTML(expresso::core::Request &req,
                                                 const std::string &localPath) {
  const std::string urlPath = req.path;
  std::string html = "<html><head><title>Index of /" + urlPath +
                     "</title></head><body><h1>Index of /" + urlPath +
                     "</h1><ul>";
  if (req.tempPath.size() != 0) {
    html += "<li><a href=\"/" + urlPath + "/..\">..</a></li>";
  }

  std::vector<std::string> files = brewtils::os::dir::list(localPath);
  std::string fileName;
  for (const std::string &file : files) {
    fileName = brewtils::string::split(file, "/").back();
    html += "<li><a href=\"/" + brewtils::os::joinPath(urlPath, fileName) +
            "\">" + fileName;
    if (brewtils::os::dir::exists(file)) {
      html += "/";
    }
    html += "</a></li>";
  }

  html += "</ul></body></html>";
  return html;
}