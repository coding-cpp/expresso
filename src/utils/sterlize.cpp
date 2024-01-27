#include <expresso/utils/sterlize.h>

const int BASE64_SIZE = 1000;
const char *BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char *HEX_DIGITS = "0123456789ABCDEF";

std::string expresso::utils::base64_encode(std::string &data) {
  std::string encoded;
  int val = 0, valb = -6;
  for (unsigned char c : data) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      encoded.push_back(BASE64_CHARS[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }

  if (valb > -6) {
    encoded.push_back(BASE64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
  }

  while (encoded.size() % 4 != 0) {
    encoded.push_back('=');
  }

  return encoded;
}

// Used from
// [GFG](https://www.geeksforgeeks.org/decode-encoded-base-64-string-ascii-string/)
std::string expresso::utils::base64_decode(std::string &data) {
  char *result = (char *)malloc(BASE64_SIZE * sizeof(char));
  int i, j, k = 0;
  int num = 0, countBits = 0;

  for (i = 0; i < data.size(); i += 4) {
    num = 0, countBits = 0;
    for (j = 0; j < 4; j++) {
      if (data[i + j] != '=') {
        num = num << 6;
        countBits += 6;
      }

      if (data[i + j] >= 'A' && data[i + j] <= 'Z')
        num = num | (data[i + j] - 'A');
      else if (data[i + j] >= 'a' && data[i + j] <= 'z')
        num = num | (data[i + j] - 'a' + 26);
      else if (data[i + j] >= '0' && data[i + j] <= '9')
        num = num | (data[i + j] - '0' + 52);
      else if (data[i + j] == '+')
        num = num | 62;
      else if (data[i + j] == '/')
        num = num | 63;
      else {
        num = num >> 2;
        countBits -= 2;
      }
    }

    while (countBits != 0) {
      countBits -= 8;
      result[k++] = (num >> countBits) & 255;
    }
  }

  result[k] = '\0';
  std::string decoded = std::string(result);

  return decoded;
}

std::string expresso::utils::url_encode(std::string &data) {
  std::string result;
  result.reserve(data.size() << 1);

  for (char ch : data) {
    if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z') || ch == '-' || ch == '_' || ch == '!' ||
        ch == '\'' || ch == '(' || ch == ')' || ch == '*' || ch == '~' ||
        ch == '.') {
      result.push_back(ch);
    } else {
      result += std::string("%") +
                HEX_DIGITS[static_cast<unsigned char>(ch) >> 4] +
                HEX_DIGITS[static_cast<unsigned char>(ch) & 15];
    }
  }

  return result;
}

std::string expresso::utils::url_decode(std::string &data) {
  std::string result;
  result.reserve(data.size());

  for (size_t i = 0; i < data.size(); i++) {
    char ch = data[i];
    if (ch == '%' && (i + 2) < data.size()) {
      std::string hex = data.substr(i + 1, 2);
      char dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
      result.push_back(dec);
      i += 2;
    } else if (ch == '+') {
      result.push_back(' ');
    } else {
      result.push_back(ch);
    }
  }

  return result;
}