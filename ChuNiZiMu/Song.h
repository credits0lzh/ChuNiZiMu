#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include "RevealResult.h"

class Song {
public:
    std::string FullSecretSongTitle;
    bool IsBonusSong = false;
    bool IsNonASCIICharacters;
    bool Affected = false;
    std::vector<uint32_t> HiddenSongTitle;
    std::unordered_set<uint32_t> RevealedCharacters;

    Song(const std::string& title, bool revealSpacesInitially = false);
    RevealResult RevealLetter(uint32_t letter);
    void RevealAll();
    std::string ToString();

private:
    std::vector<uint32_t> titleCodepoints;
    std::vector<uint32_t> lowerTitleCodepoints;
};
