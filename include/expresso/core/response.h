#pragma once

#include <netinet/in.h>
#include <sstream>

#include <brewtils/os.h>
#include <brewtils/string.h>
#include <brewtils/sys.h>
#include <json/object.h>
#include <zippuccino/zipper.h>

#include <expresso/core/cookie.h>
#include <expresso/enums/method.h>
#include <expresso/enums/status_code.h>

namespace expresso {

namespace core {

class Response {
private:
  bool hasEnded;

  int socket;
  expresso::enums::STATUS_CODE statusCode;

  std::string message;
  std::vector<Cookie *> cookies;
  std::map<std::string, std::string> headers;

  void sendToClient();
  void sendHeaders();

public:
  Response(int clientSocket);
  ~Response();

  void set(std::string headerName, std::string headerValue);
  void setCookie(Cookie *cookie);
  std::string get(std::string headerName);

  Response &status(expresso::enums::STATUS_CODE code);
  Response &send(std::string response);
  Response &json(std::string response);
  Response &json(json::object response);

  void sendFile(const std::string &path, int64_t start = -1, int64_t end = -1);
  void sendFiles(const std::set<std::string> &paths,
                 const std::string &zipFileName = "download.zip");
  void sendNotFound();
  void sendInvalidRange();

  void end();
  void print();
};

} // namespace core

} // namespace expresso