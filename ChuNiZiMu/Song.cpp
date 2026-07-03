#include "Song.h"
#include "Utf8Utils.h"
#include <algorithm>

Song::Song(const std::string& title, bool revealSpacesInitially)
    : FullSecretSongTitle(title)
{
    titleCodepoints = Utf8Utils::decode(title);
    lowerTitleCodepoints = Utf8Utils::decode(Utf8Utils::toLower(title));

    IsNonASCIICharacters = false;
    for (uint32_t cp : titleCodepoints) {
        if (cp > 0x7F) {
            IsNonASCIICharacters = true;
            break;
        }
    }

    HiddenSongTitle.resize(titleCodepoints.size());
    if (revealSpacesInitially) {
        for (size_t i = 0; i < titleCodepoints.size(); i++) {
            if (titleCodepoints[i] == ' ') {
                HiddenSongTitle[i] = ' ';
            } else {
                HiddenSongTitle[i] = '?';
            }
        }
    } else {
        std::fill(HiddenSongTitle.begin(), HiddenSongTitle.end(), '?');
    }
}

RevealResult Song::RevealLetter(uint32_t letter)
{
    // Check if already completed (mirrors original C# logic with HashSet ordering caveat)
    {
        std::vector<uint32_t> revealedVec(RevealedCharacters.begin(), RevealedCharacters.end());
        std::string revealedStr = Utf8Utils::encode(revealedVec);
        std::string lowerRevealed = Utf8Utils::toLower(revealedStr);
        std::string lowerTitle = Utf8Utils::toLower(FullSecretSongTitle);
        if (lowerRevealed == lowerTitle) {
            return RevealResult::AlreadyCompleted;
        }
    }

    letter = Utf8Utils::toLower(letter);

    if (RevealedCharacters.count(letter)) {
        return RevealResult::AlreadyRevealed;
    }

    bool inTitle = false;
    for (uint32_t cp : lowerTitleCodepoints) {
        if (cp == letter) {
            inTitle = true;
            break;
        }
    }

    if (!inTitle) {
        return RevealResult::NotInTitle;
    }

    RevealedCharacters.insert(letter);

    for (size_t i = 0; i < lowerTitleCodepoints.size(); i++) {
        if (lowerTitleCodepoints[i] == letter) {
            HiddenSongTitle[i] = letter;
        }
    }

    bool allRevealed = true;
    for (uint32_t cp : HiddenSongTitle) {
        if (cp == '?') {
            allRevealed = false;
            break;
        }
    }

    if (allRevealed) {
        HiddenSongTitle = titleCodepoints;
    }

    return RevealResult::Success;
}

void Song::RevealAll()
{
    HiddenSongTitle = titleCodepoints;
}

std::string Song::ToString()
{
    bool allRevealed = true;
    for (uint32_t cp : HiddenSongTitle) {
        if (cp == '?') {
            allRevealed = false;
            break;
        }
    }
    if (allRevealed) {
        HiddenSongTitle = titleCodepoints;
    }
    return Utf8Utils::encode(HiddenSongTitle);
}
