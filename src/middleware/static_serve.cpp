#include <expresso/middleware/static_serve.h>

expresso::middleware::StaticServe::StaticServe(std::string dirname)
    : dirname(dirname) {
  return;
}

expresso::middleware::StaticServe::~StaticServe() { return; }

bool expresso::middleware::StaticServe::use(core::Request &req,
                                            core::Response &res) {

  if (req.method != "GET") {
    return true;
  }

  std::string filePath = utils::urlDecode(req.path);
  std::string availableFile =
      utils::getAvailableFile(utils::joinPath(this->dirname, filePath));

  if (availableFile == "") {
    return true;
  }

  res.sendFile(availableFile);
  return false;
}