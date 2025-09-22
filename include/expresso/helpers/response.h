#pragma once

#include <sstream>
#include <string>

#include <brewtils/os.h>
#include <brewtils/sys.h>
#include <zippuccino/crc.h>

namespace expresso {

namespace helpers {

static const short CHUNK_SIZE = 1024;

std::string getAvailableFile(const std::string& path);

const std::string generateETag(const std::string& data);

bool sendChunkedData(const int& socket, const std::string& data);

bool sendFileInChunks(const int& socket, const std::string& path);

} // namespace helpers

} // namespace expresso