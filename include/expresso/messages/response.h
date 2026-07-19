#pragma once

#include <set>
#include <string>
#include <vector>

#include <brewtils/os.h>
#include <brewtils/string.h>
#include <json/object.h>
#include <mochios/enums/method.h>
#include <mochios/messages/response.h>
#include <zippuccino/zipper.h>

#include <expresso/enums/status_code.h>
#include <expresso/messages/cookie.h>
#include <expresso/messages/file_region.h>

namespace expresso {

namespace messages {

// Responses are serialized into an in-memory payload (plus an optional file
// region for zero-copy transfers) which the server's event loop writes to
// the client socket. A Response never touches the socket itself.
class Response : public mochios::messages::Response {
private:
  bool hasEnded;

  std::string message;
  std::string data;
  FileRegion file;
  std::vector<Cookie*> cookies;

  void serializeHeaders();

public:
  Response();
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
  std::string takeData();
  FileRegion takeFileRegion();
  const void print() const override;
};

} // namespace messages

} // namespace expresso
