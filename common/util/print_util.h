//
// Created by PC on 2026/7/3.
//

#ifndef CC_PRINT_UTIL_H
#define CC_PRINT_UTIL_H

#include <iostream>

namespace common::print {

enum class Align { kLeft, kMiddle, kRight };

// 样式
constexpr int kStyleDefault = 0;
constexpr int kStyleBold = 1;
constexpr int kStyleDim = 2;
constexpr int kStyleItalic = 3;
constexpr int kStyleUnderline = 4;
constexpr int kStyleBlink = 5;
constexpr int kStyleReverse = 7;
constexpr int kStyleHidden = 8;
constexpr int kStyleStrikethrough = 9;

// 前景色常量
constexpr int kFgBlack = 30;
constexpr int kFgRed = 31;
constexpr int kFgGreen = 32;
constexpr int kFgYellow = 33;
constexpr int kFgBlue = 34;
constexpr int kFgPurple = 35;
constexpr int kFgCyan = 36;
constexpr int kFgWhite = 37;
constexpr int kFgDefault = 39;

// 背景色常量
constexpr int kBgBlack = 40;
constexpr int kBgRed = 41;
constexpr int kBgGreen = 42;
constexpr int kBgYellow = 43;
constexpr int kBgBlue = 44;
constexpr int kBgPurple = 45;
constexpr int kBgCyan = 46;
constexpr int kBgWhite = 47;
constexpr int kBgDefault = 49;

inline std::string DisplayChar(int ch) {
    if (ch < 0 || ch >= 128) {
        return std::to_string(ch);
    }
    if (ch < 32) {  // 控制字符
        return std::to_string(ch);
    }
    // 可打印字符显示为 'x' 格式
    return '\'' + std::string(1, static_cast<char>(ch)) + '\'';
}

// ---------- 底层控制 ----------
// 开始样式设置（支持样式、前景色、背景色）
template <typename T>
std::string Styled(T val, int fg = kFgDefault, int bg = kBgDefault, int style = kStyleDefault) {
    return std::format("\033[{};{};{}m{}\033[0m", style, fg, bg, val);
}

template <typename T>
std::string PadStyled(T val,
        int width,
        Align align,
        int fg = kFgDefault,
        int bg = kBgDefault,
        int style = kStyleDefault) {
    std::string text = std::format("{}", val);
    int len = static_cast<int>(text.size());
    text = Styled(text, fg, bg, style);

    // 2. 如果宽度不足，直接应用样式返回
    if (width <= len) {
        return text;
    }

    // 3. 计算需要填充的空格数
    int pad = width - len;
    std::string padded;

    // 4. 根据对齐方式填充空格
    switch (align) {
        case Align::kLeft:
            padded = text + std::string(pad, ' ');
            break;
        case Align::kRight:
            padded = std::string(pad, ' ') + text;
            break;
        case Align::kMiddle: {
            int left = pad / 2;
            int right = pad - left;
            padded = std::string(left, ' ') + text + std::string(right, ' ');
            break;
        }
        default:
            padded = text + std::string(pad, ' ');  // 默认左对齐
            break;
    }

    // 5. 应用样式
    return padded;
}

inline void Begin(std::ostream& os, int fg, int bg = kBgDefault, int style = kStyleDefault) {
    os << Styled("", fg, bg, style);
}

inline void End(std::ostream& os) {
    os << "\033[0m";
}

}  // namespace common::print

#endif  //CC_PRINT_UTIL_H