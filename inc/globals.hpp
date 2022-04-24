#pragma once

#include <algorithm>
#include <codecvt>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <locale>
#include <numeric>
#include <sys/stat.h>
#include <thread>
#include <tuple>
#include <vector>

constexpr uint32_t WINDOW_WIDTH = 1024;
constexpr uint32_t WINDOW_HEIGHT = 768;
constexpr uint32_t FRAMERATE = 60;

constexpr uint32_t MAX_PARALLEL_DOWNLOADS = 10;

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define OS_WINDOWS
#elif defined(macintosh) || defined(Macintosh) || defined(__APPLE__) || defined(__MACH__)
#define OS_MACOS
#elif defined(__unix__) || defined(__unix) || defined(unix) || defined(__linux__) || defined(__linux)
#define OS_LINUX
#endif
