//
// Created by PC on 2026/7/3.
//

#include "file_byte_reader.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace common {

FileByteReader::FileByteReader(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("FileByteReader: cannot open file: " + file_path);
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    data_ = oss.str();
}

int FileByteReader::Read(char* dest, int start, int length) {
    if (dest == nullptr || start < 0 || length <= 0) {
        return 0;
    }
    int available = static_cast<int>(data_.size()) - next_;
    int to_read = std::min(length, available);
    if (to_read > 0) {
        std::memcpy(dest + start, data_.data() + next_, to_read);
        next_ += to_read;
    }
    return to_read;
}

int FileByteReader::ReadByte() {
    throw std::runtime_error("FileByteReader::ReadByte not implemented");
}

int FileByteReader::ReadInt() {
    throw std::runtime_error("FileByteReader::ReadInt not implemented");
}

long long FileByteReader::ReadLong() {
    throw std::runtime_error("FileByteReader::ReadLong not implemented");
}

double FileByteReader::ReadDouble() {
    throw std::runtime_error("FileByteReader::ReadDouble not implemented");
}

float FileByteReader::ReadFloat() {
    throw std::runtime_error("FileByteReader::ReadFloat not implemented");
}

bool FileByteReader::ReadBool() {
    throw std::runtime_error("FileByteReader::ReadBool not implemented");
}

short FileByteReader::ReadShort() {
    throw std::runtime_error("FileByteReader::ReadShort not implemented");
}

std::string FileByteReader::ReadString() {
    throw std::runtime_error("FileByteReader::ReadString not implemented");
}

bool FileByteReader::Available() {
    return next_ < static_cast<int>(data_.size());
}

}  // namespace common