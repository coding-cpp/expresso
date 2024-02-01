#pragma once

#include <fstream>
#include <netinet/in.h>
#include <regex>
#include <string>

#include <expresso/core/status_code.h>
#include <expresso/sys.h>
#include <expresso/utils/common.h>
#include <expresso/utils/file.h>
#include <expresso/utils/print.h>

namespace expresso {

namespace core {

class Response {
private:
  bool hasEnded;

  int socket;
  int statusCode;

  std::string message;

  std::map<std::string, std::string> headers;

  void sendToClient();
  static std::string getFileContentType(std::string fileName);

public:
  Response(int clientSocket);
  ~Response();

  void set(std::string headerName, std::string headerValue);
  std::string get(std::string headerName);

  Response status(int code);
  void send(std::string response);

  // Send absolute path to the file, not relative
  void sendFile(std::string path);

  void end();
  void print();

  static size_t CHUNK_SIZE;
};

} // namespace core

} // namespace expresso