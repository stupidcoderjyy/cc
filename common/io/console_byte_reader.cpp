//
// Created by PC on 2026/7/6.
//

#include "console_byte_reader.h"

#include <iostream>

namespace common {

ConsoleByteReader::ConsoleByteReader() : StreamByteReader(std::cin) {}

int ConsoleByteReader::Read(char* dest, int start, int length) {
    if (dest == nullptr || start < 0 || length <= 0) {
        return 0;
    }
    int count = 0;
    char ch;
    while (count < length && stream_.get(ch)) {
        if (ch == '\n') {
            // 遇到换行符，停止读取，不存储该字符
            break;
        }
        dest[start + count] = ch;
        ++count;
    }
    return count;
}

}  // namespace common
