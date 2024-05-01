#include <expresso/core/response.h>

size_t expresso::core::Response::CHUNK_SIZE = 1024;

expresso::core::Response::Response(int clientSocket)
    : socket(clientSocket), statusCode(StatusCode::OK), hasEnded(false),
      message("") {
  return;
}

expresso::core::Response::~Response() {
  if (!this->hasEnded) {
    this->sendToClient();
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

expresso::core::Response expresso::core::Response::status(int code) {
  this->statusCode = code;

  return *this;
}

expresso::core::Response expresso::core::Response::send(std::string response) {
  this->message = std::regex_replace(response, std::regex("\n"), "\r\n");
  this->message += "\r\n";
  this->set("Content-Type", "text/plain");

  return *this;
}

expresso::core::Response expresso::core::Response::json(json::object response) {
  this->message = response.dumps(0);
  this->set("Content-Type", "application/json");

  return *this;
}

expresso::core::Response
expresso::core::Response::json(const std::string response) {
  this->message = response;
  this->set("Content-Type", "application/json");

  return *this;
}

void expresso::core::Response::sendFile(std::string path) {
  std::string availableFile = utils::getAvailableFile(path);
  if (availableFile == "") {
    this->status(StatusCode::NOT_FOUND).send("Not Found");
    this->end();
    return;
  }

  std::fstream file(availableFile, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    this->status(StatusCode::NOT_FOUND).send("Not Found");
    this->end();
    return;
  }

  std::string fileName = path.substr(path.find_last_of("/") + 1);

  this->headers.erase("Content-Length");
  this->set("Transfer-Encoding", "chunked");
  this->set("Content-Type", this->getFileContentType(fileName));
  this->set("Content-Disposition", "inline; filename=\"" + fileName + "\"");
  this->set("Accept-Ranges", "bytes");
  std::string header = "HTTP/1.1 " + std::to_string(this->statusCode) + "\r\n";
  for (std::pair<const std::string, std::string> it : this->headers) {
    header += it.first + ": " + it.second + "\r\n";
  }
  for (Cookie *cookie : this->cookies) {
    header += "Set-Cookie: " + cookie->serialize() + "\r\n";
  }
  header += "\r\n";
  sys::send(this->socket, header.c_str(), header.length(), 0);

  std::string line;
  char buffer[CHUNK_SIZE];
  while (true) {
    file.read(buffer, CHUNK_SIZE);
    std::streamsize bytesRead = file.gcount();
    if (bytesRead == 0) {
      break;
    }

    std::ostringstream chunkSizeHex;
    chunkSizeHex << std::hex << bytesRead;

    std::string chunkSize = chunkSizeHex.str() + "\r\n";

    if (sys::send(this->socket, chunkSize.c_str(), chunkSize.length(), 0) < 0) {
      file.close();
      return;
    }
    if (sys::send(this->socket, buffer, bytesRead, 0) < 0) {
      file.close();
      return;
    }
    if (sys::send(this->socket, "\r\n", 2, 0) < 0) {
      file.close();
      return;
    }
  }

  sys::send(this->socket, "0\r\n\r\n", 5, 0);
  file.close();

  return;
}

void expresso::core::Response::end() {
  if (!this->hasEnded) {
    this->sendToClient();
  } else {
    logger::warning("Response has already ended");
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

  sys::send(this->socket, header.c_str(), header.length(), 0);
  sys::send(this->socket, this->message.c_str(), this->message.length(), 0);
  this->hasEnded = true;

  return;
}

std::string expresso::core::Response::getFileContentType(std::string fileName) {
  std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
  extension = utils::lower(extension);

  // Image extensions
  if (extension == "jpg" || extension == "jpeg" || extension == "png" ||
      extension == "tif" || extension == "bmp" || extension == "gif") {
    return "image/" + extension;
  } else if (extension == "svg") {
    return "image/svg+xml";
  }

  // PDF extension
  if (extension == "pdf") {
    return "application/pdf";
  }

  // Web extensions
  if (extension == "js") {
    return "application/javascript";
  } else if (extension == "css") {
    return "text/css";
  } else if (extension == "html" || extension == "htm") {
    return "text/html";
  } else if (extension == "txt") {
    return "text/plain";
  }

  // Audio extensions
  if (extension == "ogg" || extension == "wav") {
    return "audio/" + extension;
  } else if (extension == "mp3") {
    return "audio/mpeg";
  }

  // Video extensions
  // ! Not working currently, hence commented
  // if (extension == "mp4" || extension == "webm" || extension == "ogg") {
  //   return "video/" + extension;
  // } else if (extension == "avi") {
  //   return "video/x-msvideo";
  // } else if (extension == "mov") {
  //   return "video/quicktime";
  // }

  // Default -> download
  return "application/octet-stream";
}