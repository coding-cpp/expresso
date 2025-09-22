#pragma once

#include <netinet/in.h>
#include <sstream>

#include <brewtils/os.h>
#include <brewtils/string.h>
#include <brewtils/sys.h>
#include <json/object.h>
#include <mochios/enums/method.h>
#include <mochios/messages/response.h>
#include <zippuccino/zipper.h>

#include <expresso/enums/status_code.h>
#include <expresso/messages/cookie.h>

namespace expresso {

namespace messages {

class Response : public mochios::messages::Response {
private:
  bool hasEnded;

  int socket;

  std::string message;
  std::vector<Cookie*> cookies;

  void sendToClient();
  void sendHeaders();

public:
  Response(int clientSocket);
  ~Response();

  void set(std::string headerName, std::string headerValue);
  void setCookie(Cookie* cookie);
  std::string get(std::string headerName);

  Response& status(expresso::enums::STATUS_CODE code);
  Response& send(std::string response);
  Response& json(std::string response);
  Response& json(json::object response);

  void sendFile(const std::string& path, int64_t start = -1, int64_t end = -1);
  void sendFiles(const std::set<std::string>& paths,
                 const std::string& zipFileName = "download.zip");
  void sendNotFound();
  void sendInvalidRange();

  void end();
  const void print() const override;
};

} // namespace messages

} // namespace expresso