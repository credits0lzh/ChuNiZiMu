#include "Utf8Utils.h"

namespace Utf8Utils {

std::vector<uint32_t> decode(const std::string& str) {
    std::vector<uint32_t> result;
    size_t i = 0;
    while (i < str.size()) {
        uint32_t codepoint = 0;
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c <= 0x7F) {
            codepoint = c;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= str.size()) break;
            codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(str[i+1]) & 0x3F);
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= str.size()) break;
            codepoint = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(str[i+1]) & 0x3F) << 6) | (static_cast<unsigned char>(str[i+2]) & 0x3F);
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= str.size()) break;
            codepoint = ((c & 0x07) << 18) | ((static_cast<unsigned char>(str[i+1]) & 0x3F) << 12) | ((static_cast<unsigned char>(str[i+2]) & 0x3F) << 6) | (static_cast<unsigned char>(str[i+3]) & 0x3F);
            i += 4;
        } else {
            i += 1;
            continue;
        }
        result.push_back(codepoint);
    }
    return result;
}

std::string encode(const std::vector<uint32_t>& codepoints) {
    std::string result;
    for (uint32_t cp : codepoints) {
        if (cp <= 0x7F) {
            result += static_cast<char>(cp);
        } else if (cp <= 0x7FF) {
            result += static_cast<char>(0xC0 | (cp >> 6));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
            result += static_cast<char>(0xE0 | (cp >> 12));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0x10FFFF) {
            result += static_cast<char>(0xF0 | (cp >> 18));
            result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }
    return result;
}

uint32_t toLower(uint32_t cp) {
    if (cp >= 'A' && cp <= 'Z') {
        return cp + ('a' - 'A');
    }
    return cp;
}

std::string toLower(const std::string& str) {
    auto codepoints = decode(str);
    for (auto& cp : codepoints) {
        cp = toLower(cp);
    }
    return encode(codepoints);
}

bool isAscii(uint32_t cp) {
    return cp <= 0x7F;
}

} // namespace Utf8Utils
