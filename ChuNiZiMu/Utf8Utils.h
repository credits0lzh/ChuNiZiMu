#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Utf8Utils {
    std::vector<uint32_t> decode(const std::string& str);
    std::string encode(const std::vector<uint32_t>& codepoints);
    std::string toLower(const std::string& str);
    uint32_t toLower(uint32_t cp);
    bool isAscii(uint32_t cp);
}
