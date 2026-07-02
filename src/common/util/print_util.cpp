//
// Created by PC on 2026/7/3.
//

#include "print_util.h"

namespace cc {

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