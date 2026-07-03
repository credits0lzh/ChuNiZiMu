#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <cstdint>

#include "Song.h"
#include "RevealResult.h"
#include "ConsolePlus.h"
#include "ConsoleHelper.h"
#include "Utf8Utils.h"

using namespace ConsoleHelper;

static std::string readLine() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        return "";
    }
    return line;
}

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end-1]))) end--;
    return s.substr(start, end - start);
}

static bool isNullOrWhiteSpace(const std::string& s) {
    for (char c : s) {
        if (!std::isspace(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

static bool equalsIgnoreCase(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

static std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

static std::string join(const std::vector<std::string>& vec, char delimiter) {
    std::string result;
    for (size_t i = 0; i < vec.size(); i++) {
        if (i > 0) result += delimiter;
        result += vec[i];
    }
    return result;
}

static bool contains(const std::vector<std::string>& vec, const std::string& val) {
    for (const auto& s : vec) {
        if (s == val) return true;
    }
    return false;
}

static void gameMain(std::vector<Song>& songs, bool revealSpacesInitially, bool showCorrectAnswers, bool preserveAnyRevealedLetter);

static void gameInit() {
    ConsolePlus::setTitle("音游开字母(Chu Ni Zi Mu) - 根据已揭露的字符盲猜音游曲名");
    clearScreen();
    std::cout << "Welcome to Chu Ni Zi Mu, a tiny utility to manage the game which to guess the song name by the revealed characters in the song title." << std::endl;
    std::cout << "Do you want to reveal spaces initially? This settings can only be set once before the game session starts. (y/N)" << std::endl;
    std::string input = readLine();
    bool revealSpacesInitially = (trim(input) == "y" || equalsIgnoreCase(trim(input), "y"));

    std::cout << "To start the game session, please init the songs pool by input the song name once per line, and input a blank line or EOF to start the game session:" << std::endl;
    std::vector<Song> songs;
    while (true) {
        std::string songName = readLine();
        bool isBlank = isNullOrWhiteSpace(songName);
        bool isEof = equalsIgnoreCase(trim(songName), "eof");
        if (isBlank || isEof) {
            if (songs.size() < 2) {
                std::cerr << "Please at least input 2 songs to start the game session." << std::endl;
                continue;
            }
            break;
        }
        songs.emplace_back(songName, revealSpacesInitially);
    }

    std::cout << "Show correct answers during every round in the game session (for reference)?" << std::endl;
    std::cout << "This should be set true when and ONLY when just using this tool as a game backend manager, instead of a game player. (Y/n)" << std::endl;
    input = readLine();
    bool showCorrectAnswers = !(equalsIgnoreCase(trim(input), "n"));

    std::cout << "Add the revealed letter EVEN THOUGH the letter doesn't exist in any song title?" << std::endl;
    std::cout << "By enabling this feature, the revealed letter list will act better as a hint list, which was widely used in the real game chat before. (Y/n)" << std::endl;
    input = readLine();
    bool preserveAnyRevealedLetter = !(equalsIgnoreCase(trim(input), "n"));

    std::cout << "Is there a Bonus track set? Bonus tracks will be highlighted in a special color. (y/N)" << std::endl;
    input = readLine();
    bool bonusSetFlag = equalsIgnoreCase(trim(input), "y");

    if (bonusSetFlag) {
        while (true) {
            std::cout << "Please enter the Bonus track number and input a blank line or EOF to finish" << std::endl;
            std::string bonusString = readLine();
            if (bonusString.empty()) {
                std::cout << "If you want to confirm that you don't set the bonus, please enter the enter again to confirm." << std::endl;
                std::string confirmString = readLine();
                if (confirmString.empty()) {
                    break;
                }
            } else {
                std::regex re("[^0-9]+$");
                if (std::regex_search(bonusString, re)) {
                    std::cout << "Error: You entered a character other than a number, please press enter key to re-enter" << std::endl;
                    continue;
                } else {
                    auto parts = split(bonusString, ' ');
                    for (const auto& part : parts) {
                        if (part.empty()) continue;
                        try {
                            int bonusIndex = std::stoi(part);
                            if (bonusIndex > static_cast<int>(songs.size())) {
                                continue;
                            }
                            songs[bonusIndex - 1].IsBonusSong = true;
                        } catch (...) {
                            continue;
                        }
                    }
                }
                break;
            }
        }
    }

    std::cout << "Please check the following song list for the game session:" << std::endl;
    for (size_t i = 0; i < songs.size(); i++) {
        std::cout << "[" << (i + 1) << "] " << songs[i].FullSecretSongTitle << std::endl;
    }
    std::cout << "And the initial state of the game session:" << std::endl;
    for (size_t i = 0; i < songs.size(); i++) {
        std::cout << "[" << (i + 1) << "] " << Utf8Utils::encode(songs[i].HiddenSongTitle) << std::endl;
    }
    std::cout << "If check correct, press any key to start the game session. (" << songs.size() << " songs)" << std::endl;
    waitKey();
    gameMain(songs, revealSpacesInitially, showCorrectAnswers, preserveAnyRevealedLetter);
}

static void gameMain(std::vector<Song>& songs, bool revealSpacesInitially, bool showCorrectAnswers, bool preserveAnyRevealedLetter) {
    bool gameFinished = false;
    std::vector<std::string> revealedChars;
    if (revealSpacesInitially) {
        revealedChars.push_back("<空格>");
    }

    auto startTime = std::chrono::steady_clock::now();
    int round = 1;

    while (true) {
        resetColor();
        clearScreen();
        ConsolePlus::setTitle(std::string("Chu Ni Zi Mu - Round ") + (gameFinished ? "Final" : std::to_string(round)));

        // Game Main Songs Panel
        setForeground(FGColor::Yellow);
        std::cout << "已开：" << join(revealedChars, ' ') << std::endl;
        resetColor();
        std::cout << std::endl;

        for (size_t i = 0; i < songs.size(); i++) {
            Song& song = songs[i];
            if (song.ToString() == song.FullSecretSongTitle) {
                setForeground(song.IsBonusSong ? FGColor::Magenta : FGColor::Green);
                std::cout << "[" << (i + 1) << "] " << song.FullSecretSongTitle << std::endl;
                resetColor();
            } else {
                if (song.IsNonASCIICharacters) {
                    setForeground(FGColor::Blue);
                } else if (song.IsBonusSong) {
                    setForeground(FGColor::Magenta);
                }
                std::cout << "[" << (i + 1) << "] ";
                resetColor();
                std::cout << Utf8Utils::encode(song.HiddenSongTitle) << std::endl;
            }
        }

        // Game Finish logics
        if (gameFinished) {
            setBackground(BGColor::DarkBlue);
            setForeground(FGColor::White);
            round--;
            auto endTime = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
            auto hours = std::chrono::duration_cast<std::chrono::hours>(elapsed);
            auto mins = std::chrono::duration_cast<std::chrono::minutes>(elapsed % std::chrono::hours(1));
            auto secs = elapsed % std::chrono::minutes(1);

            std::cout << "Game result statistics:" << std::endl;
            std::cout << "Total rounds: " << round << std::endl;
            std::cout << "Total used time: ";
            if (hours.count() > 0) {
                std::cout << hours.count() << ":";
                std::cout << (mins.count() < 10 ? "0" : "") << mins.count() << ":";
            } else {
                std::cout << mins.count() << ":";
            }
            std::cout << (secs.count() < 10 ? "0" : "") << secs.count() << std::endl;

            std::cout << "Press any key to quit." << std::endl;
            waitKey();
            resetColor();
            return;
        }

        // Correct Answer Show logic
        if (showCorrectAnswers) {
            std::cout << std::endl;
            std::cout << "Correct answers (for reference):" << std::endl;
            for (size_t i = 0; i < songs.size(); i++) {
                std::cout << "[" << (i + 1) << "] " << songs[i].FullSecretSongTitle << std::endl;
            }
        }

        // Game Menu and Input logics
        setBackground(BGColor::DarkBlue);
        setForeground(FGColor::White);
        std::cout << "<single char> - reveal, :d <num> - directly complete a song, :q - quit" << std::endl;
        std::cout << "Input: ";
        resetColor();

        std::string option = readLine();
        option = Utf8Utils::toLower(option);

        if (option.rfind(":d", 0) == 0) {
            auto parts = split(option, ' ');
            if (parts.size() > 1) {
                try {
                    uint32_t num = std::stoul(parts[1]);
                    if (num <= songs.size()) {
                        songs[num - 1].RevealAll();
                    }
                } catch (...) {
                    // parse failed, fall through
                }
            }
        } else if (option == ":q") {
            std::cout << "Game quit." << std::endl;
            return;
        } else if (!option.empty()) {
            auto codepoints = Utf8Utils::decode(option);
            if (codepoints.size() > 1) {
                std::cout << "Only single char is allowed. Any key continue." << std::endl;
                waitKey();
                round++;
                continue;
            }
            uint32_t letter = codepoints[0];
            std::vector<RevealResult> revealResults;
            for (auto& song : songs) {
                revealResults.push_back(song.RevealLetter(letter));
            }

            bool allCompleted = true;
            for (auto result : revealResults) {
                if (result != RevealResult::AlreadyCompleted) {
                    allCompleted = false;
                    break;
                }
            }
            if (allCompleted) {
                gameFinished = true;
                round++;
                continue;
            }

            std::string shownLetter = (letter == ' ') ? "<空格>" : Utf8Utils::encode({letter});
            std::string letterStr = Utf8Utils::encode({letter});
            bool alreadyRevealed = contains(revealedChars, letterStr) ||
                                   (letter == ' ' && contains(revealedChars, "<空格>"));
            if (alreadyRevealed) {
                std::cout << "The letter " << shownLetter << " has already been revealed. Any key continue." << std::endl;
                waitKey();
                round++;
                continue;
            }

            bool noSuccess = true;
            bool hasNotInTitle = false;
            for (auto result : revealResults) {
                if (result == RevealResult::Success) noSuccess = false;
                if (result == RevealResult::NotInTitle) hasNotInTitle = true;
            }

            if (noSuccess && hasNotInTitle) {
                std::cout << "The letter " << shownLetter << " is not in any song title. Any key continue." << std::endl;
                waitKey();
                if (preserveAnyRevealedLetter) {
                    revealedChars.push_back(shownLetter);
                }
                round++;
                continue;
            }

            revealedChars.push_back(shownLetter);
        }

        round++;
    }
}

int main(int argc, char* argv[]) {
    ConsoleHelper::init();

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    for (const auto& arg : args) {
        if (arg == "--help") {
            std::cout << "Chu Ni Zi Mu is a tiny utility to manage the game which to guess the song name by the revealed characters in the song title.\n"
                      << "Usage: chunizimu [options]\n"
                      << "\n"
                      << "Options:"
                      << "--help \tShow this help message and exit.\n"
                      << "<no options> \tStart the game session." << std::endl;
            return 0;
        }
    }

    if (!args.empty()) {
        std::cerr << "Unknown argument: " << args[0] << ".\n"
                  << "Use chunizimu --help to show help message." << std::endl;
        return 1;
    }

    gameInit();
    return 0;
}
