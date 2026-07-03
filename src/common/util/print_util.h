//
// Created by PC on 2026/7/3.
//

#ifndef CC_PRINT_UTIL_H
#define CC_PRINT_UTIL_H

#include <iostream>

namespace cc {

// 前景色常量
inline constexpr int BLACK = 30;
inline constexpr int RED = 31;
inline constexpr int GREEN = 32;
inline constexpr int YELLOW = 33;
inline constexpr int BLUE = 34;
inline constexpr int PURPLE = 35;
inline constexpr int CYAN = 36;
inline constexpr int WHITE = 37;

// 背景色常量
inline constexpr int BG_BLACK = 40;
inline constexpr int BG_RED = 41;
inline constexpr int BG_GREEN = 42;
inline constexpr int BG_YELLOW = 43;
inline constexpr int BG_BLUE = 44;
inline constexpr int BG_PURPLE = 45;
inline constexpr int BG_CYAN = 46;
inline constexpr int BG_WHITE = 47;

std::string DisplayChar(int ch);

// ---------- 底层控制 ----------
// 开始样式设置（支持样式、前景色、背景色）
void Begin(int style, int fg, int bg, std::ostream& os = std::cout);
// 开始样式设置（支持样式、前景色）
void Begin(int style, int fg, std::ostream& os = std::cout);
// 开始样式设置（仅样式）
void Begin(int style, std::ostream& os = std::cout);
// 重置所有属性
void End(std::ostream& os = std::cout);

// ---------- 通用打印（模板支持任意可流输出类型） ----------
template <typename T>
void PrintNormal(int fg, const T& info, std::ostream& os = std::cout) {
    Begin(0, fg, os);
    os << info;
    End(os);
}

template <typename T>
void PrintStyled(int style, int fg, const T& info, std::ostream& os = std::cout) {
    Begin(style, fg, os);
    os << info;
    End(os);
}

template <typename T>
void PrintComplex(int style, int fg, int bg, const T& info, std::ostream& os = std::cout) {
    Begin(style, fg, bg, os);
    os << info;
    End(os);
}

// ---------- 便捷单色打印 ----------
template <typename T>
void PrintRed(const T& info, std::ostream& os = std::cout) {
    PrintNormal(RED, info, os);
}

template <typename T>
void PrintGreen(const T& info, std::ostream& os = std::cout) {
    PrintNormal(GREEN, info, os);
}

template <typename T>
void PrintYellow(const T& info, std::ostream& os = std::cout) {
    PrintNormal(YELLOW, info, os);
}

template <typename T>
void PrintBlue(const T& info, std::ostream& os = std::cout) {
    PrintNormal(BLUE, info, os);
}

template <typename T>
void PrintPurple(const T& info, std::ostream& os = std::cout) {
    PrintNormal(PURPLE, info, os);
}

template <typename T>
void PrintCyan(const T& info, std::ostream& os = std::cout) {
    PrintNormal(CYAN, info, os);
}

// ---------- 高亮（背景色）打印 ----------
template <typename T>
void PrintHighlightWhite(const T& info, std::ostream& os = std::cout) {
    Begin(7, os);  // 7 表示反白（高亮）模式
    os << info;
    End(os);
}

template <typename T>
void PrintHighlightPurple(const T& info, std::ostream& os = std::cout) {
    PrintComplex(0, BLACK, BG_PURPLE, info, os);
}

template <typename T>
void PrintHighlightRed(const T& info, std::ostream& os = std::cout) {
    PrintComplex(0, BLACK, BG_RED, info, os);
}

template <typename T>
void PrintHighlightYellow(const T& info, std::ostream& os = std::cout) {
    PrintComplex(0, BLACK, BG_YELLOW, info, os);
}

template <typename T>
void PrintHighlightBlue(const T& info, std::ostream& os = std::cout) {
    PrintComplex(0, BLACK, BG_BLUE, info, os);
}

template <typename T>
void PrintHighlightGreen(const T& info, std::ostream& os = std::cout) {
    PrintComplex(0, BLACK, BG_GREEN, info, os);
}

template <typename T>
void PrintHighlightCyan(const T& info, std::ostream& os = std::cout) {
    PrintComplex(0, BLACK, BG_CYAN, info, os);
}

}  // namespace cc

#endif  //CC_PRINT_UTIL_H