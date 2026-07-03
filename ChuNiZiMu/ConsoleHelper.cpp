#include "ConsoleHelper.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#endif

namespace ConsoleHelper {

#ifdef _WIN32
static WORD s_defaultAttr = 0x07;

static WORD fgToWinAttr(FGColor color) {
    switch (color) {
        case FGColor::Yellow:  return 14;
        case FGColor::Green:   return 10;
        case FGColor::Magenta: return 13;
        case FGColor::Blue:    return 9;
        case FGColor::White:   return 15;
        default:               return 7;
    }
}

static WORD bgToWinAttr(BGColor color) {
    switch (color) {
        case BGColor::DarkBlue: return 1;
        default:                return 0;
    }
}
#endif

void init() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        s_defaultAttr = csbi.wAttributes;
    }
#endif
}

void setForeground(FGColor color) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    WORD bg = (csbi.wAttributes >> 4) & 0x0F;
    if (color == FGColor::Default) {
        SetConsoleTextAttribute(hOut, (bg << 4) | 7);
    } else {
        SetConsoleTextAttribute(hOut, (bg << 4) | fgToWinAttr(color));
    }
#else
    const char* code = "39";
    switch (color) {
        case FGColor::Yellow:  code = "93"; break;
        case FGColor::Green:   code = "92"; break;
        case FGColor::Magenta: code = "95"; break;
        case FGColor::Blue:    code = "94"; break;
        case FGColor::White:   code = "97"; break;
        default:               code = "39"; break;
    }
    std::cout << "\033[" << code << "m";
    std::cout.flush();
#endif
}

void setBackground(BGColor color) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    WORD fg = csbi.wAttributes & 0x0F;
    if (color == BGColor::Default) {
        SetConsoleTextAttribute(hOut, fg);
    } else {
        SetConsoleTextAttribute(hOut, (bgToWinAttr(color) << 4) | fg);
    }
#else
    const char* code = "49";
    switch (color) {
        case BGColor::DarkBlue: code = "44"; break;
        default:                code = "49"; break;
    }
    std::cout << "\033[" << code << "m";
    std::cout.flush();
#endif
}

void resetColor() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, s_defaultAttr);
#else
    std::cout << "\033[0m";
    std::cout.flush();
#endif
}

void clearScreen() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD homeCoords = {0, 0};
    DWORD charsWritten;
    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, homeCoords, &charsWritten);
    FillConsoleOutputAttribute(hConsole, s_defaultAttr, consoleSize, homeCoords, &charsWritten);
    SetConsoleCursorPosition(hConsole, homeCoords);
#else
    std::cout << "\033[2J\033[H";
    std::cout.flush();
#endif
}

int waitKey() {
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

bool isStdinEOF() {
#ifdef _WIN32
    // On Windows, Console.ReadLine returns null on Ctrl+Z
    // We detect it via std::cin failing
    return std::cin.eof();
#else
    return std::cin.eof();
#endif
}

} // namespace ConsoleHelper
