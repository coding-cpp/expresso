#include <expresso/helpers/response.h>

std::string expresso::helpers::getAvailableFile(const std::string &path) {
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

const std::string expresso::helpers::generateETag(const std::string &data) {
  const std::string availableFile = expresso::helpers::getAvailableFile(data);
  uint32_t crc = zippuccino::crc::compute(availableFile);
  std::ostringstream etag;
  etag << "\"" << std::hex << std::setw(8) << std::setfill('0') << crc << "\"";
  return etag.str();
}

bool expresso::helpers::sendChunkedData(const int &socket,
                                        const std::string &data) {
  std::ostringstream dataSizeHex;
  dataSizeHex << std::hex << data.length();
  std::string dataSize = dataSizeHex.str() + "\r\n";

  if (brewtils::sys::send(socket, dataSize.c_str(), dataSize.length(), 0) ==
      -1) {
    return false;
  }
  if (brewtils::sys::send(socket, data.c_str(), data.length(), 0) == -1) {
    return false;
  }
  if (brewtils::sys::send(socket, "\r\n", 2, 0) == -1) {
    return false;
  }
  return true;
}

bool expresso::helpers::sendFileInChunks(const int &socket,
                                         const std::string &path) {
  std::fstream file(path, std::ios::in | std::ios::binary);
  char buffer[expresso::helpers::CHUNK_SIZE];

  try {
    std::streamsize bytesRead = 0;
    while (true) {
      file.read(buffer, expresso::helpers::CHUNK_SIZE);
      bytesRead = file.gcount();
      if (bytesRead == 0) {
        break;
      }

      if (!expresso::helpers::sendChunkedData(socket,
                                              std::string(buffer, bytesRead))) {
        return false;
      }
    }
  } catch (const std::exception &e) {
    logger::error(e.what(), "bool sendFileInChunks(const int &socket, const "
                            "std::string &path)");
    return false;
  }
  if (file.is_open()) {
    file.close();
  }

  return true;
}