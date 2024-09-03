#include <expresso/core/response.h>
#include <expresso/helpers/response.h>

expresso::core::Response::Response(int clientSocket)
    : hasEnded(false), socket(clientSocket),
      statusCode(expresso::enums::STATUS_CODE::OK), message("") {
  return;
}

expresso::core::Response::~Response() {
  if (!this->hasEnded) {
    this->sendToClient();
  }

  for (expresso::core::Cookie *cookie : this->cookies) {
    delete cookie;
  }

  return;
}

void expresso::core::Response::set(std::string headerName,
                                   std::string headerValue) {
  this->headers[headerName] = headerValue;

  return;
}

void expresso::core::Response::setCookie(Cookie *cookie) {
  this->cookies.push_back(cookie);

  return;
}

std::string expresso::core::Response::get(std::string headerName) {
  if (this->headers.find(headerName) != this->headers.end()) {
    return this->headers[headerName];
  } else {
    return "";
  }
}

expresso::core::Response &
expresso::core::Response::status(expresso::enums::STATUS_CODE code) {
  this->statusCode = code;

  return *this;
}

expresso::core::Response &expresso::core::Response::send(std::string response) {
  this->message = brewtils::string::replace(response, "\n", "\r\n");
  this->message += "\r\n";
  this->set("Content-Type", "text/plain");

  return *this;
}

expresso::core::Response &expresso::core::Response::json(std::string response) {
  this->message = response;
  this->set("Content-Type", "application/json");

  return *this;
}

expresso::core::Response &
expresso::core::Response::json(json::object response) {
  this->message = response.dumps(0);
  this->set("Content-Type", "application/json");

  return *this;
}

void expresso::core::Response::sendFile(const std::string &path, int64_t start,
                                        int64_t end) {
  std::string availableFile = expresso::helpers::getAvailableFile(path);
  if (availableFile.empty() || !brewtils::os::file::exists(availableFile)) {
    return this->sendNotFound();
  }

  int64_t fileSize = brewtils::os::file::size(availableFile);
  if (start >= fileSize || end >= fileSize) {
    return this->sendInvalidRange();
  }

  bool isPartial = start >= 0 || end >= 0;
  std::string fileName =
      availableFile.substr(availableFile.find_last_of('/') + 1);
  this->set("Content-Type", brewtils::os::file::getMimeType(fileName));
  this->set("Content-Disposition", "inline; filename=\"" + fileName + "\"");
  this->set("Accept-Ranges", "bytes");

  if (!isPartial) {
    start = 0;
    end = fileSize - 1;
    this->set("Content-Length", std::to_string(fileSize));
  } else {
    if (start < 0) {
      start = 0;
    }
    if (end < 0) {
      end = fileSize - 1;
    }
    this->status(expresso::enums::STATUS_CODE::PARTIAL_CONTENT);
    this->set("Content-Length", std::to_string(end - start + 1));
    this->set("Content-Range", "bytes " + std::to_string(start) + "-" +
                                   std::to_string(end) + "/" +
                                   std::to_string(fileSize));
  }
  this->sendHeaders();

  std::ifstream file(availableFile, std::ios::binary);
  try {
    file.seekg(start, std::ios::beg);
    char buffer[expresso::helpers::CHUNK_SIZE];
    while (file.read(buffer, expresso::helpers::CHUNK_SIZE)) {
      if (brewtils::sys::send(this->socket, buffer,
                              expresso::helpers::CHUNK_SIZE, 0) == -1) {
        this->hasEnded = true;
        break;
      }
    }
    if (file.gcount() > 0) {
      if (brewtils::sys::send(this->socket, buffer, file.gcount(), 0) == -1) {
        this->hasEnded = true;
      }
    }
  } catch (const std::exception &e) {
    logger::error(e.what(), "void expresso::core::Response::sendFile(const "
                            "std::string &path, int64_t start, int64_t end)");
  }
  if (file.is_open()) {
    file.close();
  }

  return;
}

void expresso::core::Response::sendFiles(const std::set<std::string> &paths,
                                         const std::string &zipFileName) {
  this->headers.erase("Content-Length");
  this->set("Transfer-Encoding", "chunked");
  this->set("Content-Type", brewtils::os::file::getMimeType(zipFileName));
  this->set("Content-Disposition", "inline; filename=\"" + zipFileName + "\"");
  this->set("Accept-Ranges", "bytes");
  this->sendHeaders();

  zippuccino::Zipper zipper;
  for (const std::string &path : paths) {
    zipper.add(path);
  }
  zipper.zip();

  try {
    while (!zipper.isFinished()) {
      if (!expresso::helpers::sendChunkedData(this->socket,
                                              zipper.getHeader())) {
        this->hasEnded = true;
        return;
      }
      std::string currentFile = zipper.getCurrentFile();
      if (!expresso::helpers::sendFileInChunks(this->socket, currentFile)) {
        this->hasEnded = true;
        return;
      }
    }

    if (!expresso::helpers::sendChunkedData(this->socket, zipper.getFooter())) {
      this->hasEnded = true;
      return;
    }
    if (brewtils::sys::send(this->socket, "0\r\n\r\n", 5, 0) == -1) {
      this->hasEnded = true;
    }
  } catch (const std::exception &e) {
    logger::error(e.what(), "void expresso::core::Response::sendFiles(const "
                            "std::set<std::string> &paths, const std::string "
                            "&zipFileName)");
  }

  return;
}

void expresso::core::Response::sendNotFound() {
  this->status(expresso::enums::STATUS_CODE::NOT_FOUND).send("Not Found").end();

  return;
}

void expresso::core::Response::sendInvalidRange() {
  this->set("Content-Range", "bytes */");
  this->set("Connection", "close");
  this->status(expresso::enums::STATUS_CODE::RANGE_NOT_SATISFIABLE)
      .send("Invalid Range")
      .end();

  return;
}

void expresso::core::Response::end() {
  if (!this->hasEnded) {
    this->sendToClient();
  }

  return;
}

void expresso::core::Response::print() {
  logger::info("Response: ");
  logger::info("  statusCode: " + std::to_string(this->statusCode));
  logger::info("  headers: ");
  for (const std::pair<const std::string, std::string> &header :
       this->headers) {
    logger::info("    " + header.first + ": " + header.second);
  }
  logger::info("  message: " + this->message);

  return;
}

void expresso::core::Response::sendToClient() {
  this->set("Content-Length", std::to_string(this->message.length()));
  this->sendHeaders();
  if (this->hasEnded) {
    return;
  }
  brewtils::sys::send(this->socket, this->message.c_str(),
                      this->message.length(), 0);
  this->hasEnded = true;

  return;
}

void expresso::core::Response::sendHeaders() {
  std::string header = "HTTP/1.1 " + std::to_string(this->statusCode) + "\r\n";
  for (std::pair<const std::string, std::string> it : this->headers) {
    header += it.first + ": " + it.second + "\r\n";
  }
  for (Cookie *cookie : this->cookies) {
    header += "Set-Cookie: " + cookie->serialize() + "\r\n";
  }
  header += "\r\n";
  if (brewtils::sys::send(this->socket, header.c_str(), header.length(), 0) ==
      -1) {
    this->hasEnded = true;
  }
  return;
}
