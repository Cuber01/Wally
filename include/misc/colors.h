#ifndef WALLY_COLORS_H
#define WALLY_COLORS_H

#include "common.h"

#ifdef NO_COLORS

#define BLACK  ""
#define RED    ""
#define GREEN  ""
#define YELLOW ""
#define BLUE   ""
#define PURPLE ""
#define CYAN   ""
#define WHITE  ""

#define BOLD_BLACK  ""
#define BOLD_RED    ""
#define BOLD_GREEN  ""
#define BOLD_YELLOW ""
#define BOLD_BLUE   ""
#define BOLD_PURPLE ""
#define BOLD_CYAN   ""
#define BOLD_WHITE  ""

#define COLOR_CLEAR ""

#else

#define BLACK  "\e[0;30m"
#define RED    "\e[0;31m"
#define GREEN  "\e[0;32m"
#define YELLOW "\e[0;33m"
#define BLUE   "\e[0;34m"
#define PURPLE "\e[0;35m"
#define CYAN   "\e[0;36m"
#define WHITE  "\e[0;37m"

#define BOLD_BLACK  "\e[1;30m"
#define BOLD_RED    "\e[1;31m"
#define BOLD_GREEN  "\e[1;32m"
#define BOLD_YELLOW "\e[1;33m"
#define BOLD_BLUE   "\e[1;34m"
#define BOLD_PURPLE "\e[1;35m"
#define BOLD_CYAN   "\e[1;36m"
#define BOLD_WHITE  "\e[1;37m"

#define COLOR_CLEAR "\e[0m"

#endif

void colorWriteLine(const char* colorCode, const char* format, ...);
void colorWrite(const char* colorCode, const char* format, ...);
void stderrPrint(const char* format, ...);

#endif //WALLY_COLORS_H
