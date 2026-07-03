#pragma once
#include <string>

namespace ConsoleHelper {
    enum class FGColor {
        Default,
        Yellow,
        Green,
        Magenta,
        Blue,
        White
    };

    enum class BGColor {
        Default,
        DarkBlue
    };

    void init();
    void setForeground(FGColor color);
    void setBackground(BGColor color);
    void resetColor();
    void clearScreen();
    int waitKey();
    bool isStdinEOF();
}
