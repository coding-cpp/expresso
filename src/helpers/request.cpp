#include <brewtils/string.h>

#include <expresso/helpers/request.h>

bool expresso::helpers::isRequestComplete(const std::string& request) {
  size_t headersEnd = request.find("\r\n\r\n");
  if (headersEnd == std::string::npos) {
    return false;
  }

  size_t bodyStart = headersEnd + 4;
  std::string headers =
      brewtils::string::lower(request.substr(0, headersEnd + 2));

  size_t contentLength = 0;
  size_t position = headers.find("\r\ncontent-length:");
  if (position != std::string::npos) {
    size_t valueStart = headers.find(':', position) + 1;
    size_t valueEnd = headers.find('\r', valueStart);
    try {
      contentLength = std::stoull(brewtils::string::trim(
          headers.substr(valueStart, valueEnd - valueStart)));
    } catch (const std::exception&) {
      contentLength = 0;
    }
  }

  return request.size() - bodyStart >= contentLength;
}
