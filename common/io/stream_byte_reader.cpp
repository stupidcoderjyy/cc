//
// Created by PC on 2026/7/6.
//

#include "stream_byte_reader.h"

#include <stdexcept>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>

#include "console_byte_reader.h"

namespace common {

// ---------- 构造函数 ----------
StreamByteReader::StreamByteReader(std::istream& stream) : stream_(stream) {}

StreamByteReader::StreamByteReader(std::unique_ptr<std::istream> stream): stream_(*stream), owned_stream_(std::move(stream)) {}

std::unique_ptr<StreamByteReader> StreamByteReader::FromString(const std::string &str) {
    return std::make_unique<StreamByteReader>(std::make_unique<std::istringstream>(str));
}

std::unique_ptr<StreamByteReader> StreamByteReader::FromFile(const std::string &path) {
    auto ifs = std::make_unique<std::ifstream>(path, std::ios::binary);
    if (!ifs->is_open()) {
        throw std::runtime_error("StreamByteReader::FromFile: cannot open file: " + path);
    }
    return std::make_unique<StreamByteReader>(std::move(ifs));
}

std::unique_ptr<StreamByteReader> StreamByteReader::FromConsole() {
    return std::make_unique<ConsoleByteReader>();
}

// ---------- Read (原始字节) ----------
int StreamByteReader::Read(char* dest, int start, int length) {
    if (!Available()) {
        return 0;
    }
    if (dest == nullptr || start < 0 || length <= 0) {
        return 0;
    }
    stream_.read(dest + start, length);
    return static_cast<int>(stream_.gcount());
}

int StreamByteReader::ReadByte() {
    char ch;
    if (Read(&ch, 0, 1) != 1) {
        throw std::runtime_error("StreamByteReader::ReadByte: failed to read byte");
    }
    return static_cast<unsigned char>(ch);
}

int StreamByteReader::ReadInt() {
    std::int32_t value;
    if (Read(reinterpret_cast<char*>(&value), 0, sizeof(value)) != sizeof(value)) {
        throw std::runtime_error("StreamByteReader::ReadInt: failed to read int");
    }
    return value;
}

long long StreamByteReader::ReadLong() {
    std::int64_t value;
    if (Read(reinterpret_cast<char*>(&value), 0, sizeof(value)) != sizeof(value)) {
        throw std::runtime_error("StreamByteReader::ReadLong: failed to read long long");
    }
    return value;
}

double StreamByteReader::ReadDouble() {
    double value;
    if (Read(reinterpret_cast<char*>(&value), 0, sizeof(value)) != sizeof(value)) {
        throw std::runtime_error("StreamByteReader::ReadDouble: failed to read double");
    }
    return value;
}

float StreamByteReader::ReadFloat() {
    float value;
    if (Read(reinterpret_cast<char*>(&value), 0, sizeof(value)) != sizeof(value)) {
        throw std::runtime_error("StreamByteReader::ReadFloat: failed to read float");
    }
    return value;
}

bool StreamByteReader::ReadBool() {
    unsigned char byte;
    if (Read(reinterpret_cast<char*>(&byte), 0, 1) != 1) {
        throw std::runtime_error("StreamByteReader::ReadBool: failed to read bool");
    }
    return byte != 0;
}

short StreamByteReader::ReadShort() {
    std::int16_t value;
    if (Read(reinterpret_cast<char*>(&value), 0, sizeof(value)) != sizeof(value)) {
        throw std::runtime_error("StreamByteReader::ReadShort: failed to read short");
    }
    return value;
}

std::string StreamByteReader::ReadString() {
    std::int32_t len;
    if (Read(reinterpret_cast<char*>(&len), 0, sizeof(len)) != sizeof(len)) {
        throw std::runtime_error("StreamByteReader::ReadString: failed to read length");
    }
    if (len < 0) {
        throw std::runtime_error("StreamByteReader::ReadString: negative string length");
    }
    if (len == 0) {
        return {};
    }
    std::string str(len, '\0');
    if (Read(&str[0], 0, len) != len) {
        throw std::runtime_error("StreamByteReader::ReadString: failed to read string data");
    }
    return str;
}

// ---------- 检查是否可用 ----------
bool StreamByteReader::Available() {
    return stream_.good() && stream_.rdbuf()->in_avail() > 0;
}
}  // namespace common