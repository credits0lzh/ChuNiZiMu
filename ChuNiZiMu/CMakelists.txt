cmake_minimum_required(VERSION 3.10)
project(ChuNiZiMu CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ChuNiZiMu)

add_executable(chunizimu
    ChuNiZiMu/main.cpp
    ChuNiZiMu/Utf8Utils.cpp
    ChuNiZiMu/Song.cpp
    ChuNiZiMu/ConsolePlus.cpp
    ChuNiZiMu/ConsoleHelper.cpp
)

if(WIN32)
    target_compile_definitions(chunizimu PRIVATE _WIN32_WINNT=0x0501 WINVER=0x0501)
    target_link_libraries(chunizimu PRIVATE kernel32)
endif()

if(MINGW)
    # Windows MinGW static
    target_link_options(chunizimu PRIVATE -static -static-libgcc -static-libstdc++)
else()
    # Android NDK & Linux static
    target_link_options(chunizimu PRIVATE -static)
endif()
