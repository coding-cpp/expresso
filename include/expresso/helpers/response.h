#pragma once

#include <string>

#include <brewtils/os.h>
#include <zippuccino/crc.h>

namespace expresso {

namespace helpers {

static const short CHUNK_SIZE = 1024;

std::string getAvailableFile(const std::string& path);

const std::string generateETag(const std::string& data);

std::string makeChunkedData(const std::string& data);

std::string readFileAsChunked(const std::string& path);

} // namespace helpers

} // namespace expresso
