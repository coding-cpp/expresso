#include <expresso/core/response.h>

namespace expresso::core {

size_t Response::CHUNK_SIZE = 1024;

std::string Response::NOT_FOUND = "Not Found";

std::map<std::string, std::set<std::string>> Response::MIME_TYPES = {
    {"text", {"htm", "html", "css", "log", "md", "csv"}},
    {"image",
     {"png", "jpg", "jpeg", "gif", "tif", "bmp", "webp", "apng", "avif"}},
    {"application", {"pdf", "json", "zip", "rtf", "xml"}},
    {"audio", {"mp3", "wav", "aac", "wma", "midi"}},
    {"video", {"mp4", "webm", "ogg"}},
    {"font", {"ttf", "otf", "woff"}}};

} // namespace expresso::core

expresso::core::Response::Response(int clientSocket)
    : hasEnded(false), socket(clientSocket),
      statusCode(expresso::core::STATUS_CODE::OK), message("") {
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
expresso::core::Response::status(expresso::core::STATUS_CODE code) {
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

void expresso::core::Response::sendFile(std::string &path) {
  std::string availableFile = this->getAvailableFile(path);
  if (availableFile.empty()) {
    return this->sendNotFound();
  }

  std::fstream file(availableFile, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    return this->sendNotFound();
  }

  std::string fileName = path.substr(path.find_last_of('/') + 1);
  this->headers.erase("Content-Length");
  this->set("Transfer-Encoding", "chunked");
  this->set("Content-Type", this->getMimeType(fileName));
  this->set("Content-Disposition", "inline; filename=\"" + fileName + "\"");
  this->set("Accept-Ranges", "bytes");
  std::string headers = "HTTP/1.1 " + std::to_string(this->statusCode) + "\r\n";
  for (std::pair<const std::string, std::string> &header : this->headers) {
    headers += header.first + ": " + header.second + "\r\n";
  }
  for (expresso::core::Cookie *cookie : this->cookies) {
    headers += "Set-Cookie: " + cookie->serialize() + "\r\n";
  }
  headers += "\r\n";
  brewtils::sys::send(this->socket, headers.c_str(), headers.length(), 0);

  std::string line;
  char buffer[expresso::core::Response::CHUNK_SIZE];
  while (true) {
    file.read(buffer, expresso::core::Response::CHUNK_SIZE);
    std::streamsize bytesRead = file.gcount();
    if (bytesRead == 0) {
      break;
    }

    std::ostringstream chunkSizeHex;
    chunkSizeHex << std::hex << bytesRead;
    std::string chunkSize = chunkSizeHex.str() + "\r\n";
    if (brewtils::sys::send(this->socket, chunkSize.c_str(), chunkSize.length(),
                            0) == -1) {
      file.close();
      break;
    }
    if (brewtils::sys::send(this->socket, buffer, bytesRead, 0) == -1) {
      file.close();
      break;
    }
    if (brewtils::sys::send(this->socket, "\r\n", 2, 0) == -1) {
      file.close();
      break;
    }
  }

  brewtils::sys::send(this->socket, "0\r\n\r\n", 5, 0);
  file.close();
  return;
}

void expresso::core::Response::sendNotFound() {
  this->status(expresso::core::STATUS_CODE::NOT_FOUND)
      .send(expresso::core::Response::NOT_FOUND)
      .end();

  return;
}

void expresso::core::Response::end() {
  if (!this->hasEnded) {
    this->sendToClient();
  } else {
    logger::warning("Response has already ended.");
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
  std::string header = "HTTP/1.1 " + std::to_string(this->statusCode) + "\r\n";
  this->set("Content-Length", std::to_string(this->message.length()));
  for (std::pair<const std::string, std::string> it : this->headers) {
    header += it.first + ": " + it.second + "\r\n";
  }
  for (Cookie *cookie : this->cookies) {
    header += "Set-Cookie: " + cookie->serialize() + "\r\n";
  }
  header += "\r\n";

  brewtils::sys::send(this->socket, header.c_str(), header.length(), 0);
  brewtils::sys::send(this->socket, this->message.c_str(),
                      this->message.length(), 0);
  this->hasEnded = true;

  return;
}

std::string
expresso::core::Response::getAvailableFile(const std::string &path) {
  if (brewtils::os::file::exists(path)) {
    return path;
  }

  std::string tempPath = brewtils::os::joinPath(path, "index.htm");
  if (brewtils::os::file::exists(tempPath)) {
    return tempPath;
  }

  tempPath.push_back('l');
  if (brewtils::os::file::exists(tempPath)) {
    return tempPath;
  }

  return "";
}

std::string expresso::core::Response::getMimeType(const std::string &path) {
  std::string extension = path.substr(path.find_last_of('.') + 1);
  extension = brewtils::string::lower(extension);

  // Default MIME types
  for (const std::pair<const std::string, std::set<std::string>> &it :
       expresso::core::Response::MIME_TYPES) {
    if (it.second.find(extension) != it.second.end()) {
      return it.first + "/" + extension;
    }
  }

  // Special cases handling
  if (extension == "js") {
    return "text/javascript";
  } else if (extension == "svg") {
    return "image/svg+xml";
  } else if (extension == "ico") {
    return "image/x-icon";
  } else if (extension == "txt") {
    return "text/plain";
  } else if (extension == "mp3") {
    return "audio/mpeg";
  }

  // Default MIME type -> download
  return "application/octet-stream";
}
