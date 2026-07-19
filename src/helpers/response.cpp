#include <fstream>
#include <iomanip>
#include <sstream>

#include <logger/log.h>

#include <expresso/helpers/response.h>

std::string expresso::helpers::getAvailableFile(const std::string& path) {
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

const std::string expresso::helpers::generateETag(const std::string& data) {
  const std::string availableFile = expresso::helpers::getAvailableFile(data);
  uint32_t crc = zippuccino::crc::compute(availableFile);
  std::ostringstream etag;
  etag << "\"" << std::hex << std::setw(8) << std::setfill('0') << crc << "\"";
  return etag.str();
}

std::string expresso::helpers::makeChunkedData(const std::string& data) {
  std::ostringstream chunk;
  chunk << std::hex << data.length() << "\r\n" << data << "\r\n";
  return chunk.str();
}

std::string expresso::helpers::readFileAsChunked(const std::string& path) {
  std::string chunks;
  std::fstream file(path, std::ios::in | std::ios::binary);
  char buffer[expresso::helpers::CHUNK_SIZE];

  try {
    while (true) {
      file.read(buffer, expresso::helpers::CHUNK_SIZE);
      std::streamsize bytesRead = file.gcount();
      if (bytesRead == 0) {
        break;
      }
      chunks += expresso::helpers::makeChunkedData(
          std::string(buffer, static_cast<size_t>(bytesRead)));
    }
  } catch (const std::exception& e) {
    logger::error(e.what(),
                  "std::string expresso::helpers::readFileAsChunked(const "
                  "std::string &path)");
  }

  if (file.is_open()) {
    file.close();
  }

  return chunks;
}
