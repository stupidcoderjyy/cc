//
// Created by PC on 2026/7/3.
//

#include "print_util.h"

namespace cc {

std::string DisplayChar(int ch) {
    if (ch < 0 || ch >= 128) {
        return std::to_string(ch);
    }
    if (ch < 32) {  // 控制字符
        return std::to_string(ch);
    }
    // 可打印字符显示为 'x' 格式
    return {static_cast<char>(ch)};
}

// ---------- 底层实现 ----------
void Begin(int style, int fg, int bg, std::ostream& os) {
    os << "\033[" << style << ';' << fg << ';' << bg << 'm';
}

void Begin(int style, int fg, std::ostream& os) {
    os << "\033[" << style << ';' << fg << 'm';
}

void Begin(int style, std::ostream& os) {
    os << "\033[" << style << 'm';
}

void End(std::ostream& os) {
    os << "\033[0m";
}

}  // namespace cc