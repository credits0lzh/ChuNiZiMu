#include "ConsolePlus.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ConsolePlus {

void setTitle(const std::string& title) {
#ifdef _WIN32
    int wlen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
    if (wlen > 0) {
        std::wstring wtitle(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wtitle[0], wlen);
        SetConsoleTitleW(wtitle.c_str());
    }
#else
    std::cout << "\033]0;" << title << "\007";
    std::cout.flush();
#endif
}

} // namespace ConsolePlus
