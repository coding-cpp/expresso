#pragma once

#include <netinet/in.h>
#include <sstream>

#include <brewtils/os.h>
#include <brewtils/string.h>
#include <brewtils/sys.h>
#include <json/object.h>
#include <zippuccino/zipper.h>

#include <expresso/core/cookie.h>
#include <expresso/core/status_code.h>

namespace expresso {

namespace core {

class Response {
private:
  bool hasEnded;

  int socket;
  expresso::core::STATUS_CODE statusCode;

  std::string message;
  std::vector<Cookie *> cookies;
  std::map<std::string, std::string> headers;

  bool sendDataInChunks(const std::string &data);
  void sendFileInChunks(const std::string &path);
  void sendToClient();

  static std::string getMimeType(const std::string &path);

public:
  Response(int clientSocket);
  ~Response();

  void set(std::string headerName, std::string headerValue);
  void setCookie(Cookie *cookie);
  std::string get(std::string headerName);

  Response &status(expresso::core::STATUS_CODE code);
  Response &send(std::string response);
  Response &json(std::string response);
  Response &json(json::object response);

  // Send the absolute path to the file, not the relative path.
  void sendFile(std::string &path);
  void sendFiles(const std::set<std::string> &paths,
                 const std::string &zipFileName = "download.zip");
  void sendNotFound();

  void end();
  void print();

  static size_t CHUNK_SIZE;
  static std::string NOT_FOUND;
  static std::map<std::string, std::set<std::string>> MIME_TYPES;

  static std::string getAvailableFile(const std::string &path);
};

} // namespace core

} // namespace expresso