#include <expresso/middleware/static_serve.h>

expresso::middleware::StaticServe::StaticServe(std::string dirname,
                                               bool showListing)
    : dirname(dirname), showListing(showListing) {
  return;
}

expresso::middleware::StaticServe::~StaticServe() { return; }

bool expresso::middleware::StaticServe::use(core::Request &req,
                                            core::Response &res) {
  if (req.method != "GET") {
    return true;
  }

  std::string filePath = brewtils::url::decode(req.tempPath);
  std::string availableFile = expresso::core::Response::getAvailableFile(
      brewtils::os::joinPath(this->dirname, filePath));

  if (availableFile == "") {
    return true;
  }

  res.sendFile(availableFile);
  return false;
}