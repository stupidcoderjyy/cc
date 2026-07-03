//
// Created by PC on 2026/7/3.
//

#ifndef RTFS2D_FILE_BYTE_READER_H
#define RTFS2D_FILE_BYTE_READER_H

#include <string>

#include "byte_reader.h"

namespace common {

// 基于文件的字节读取器（将整个文件读入内存）
class FileByteReader final : public ByteReader {
public:
    // 从文件路径构造
    explicit FileByteReader(const std::string& file_path);

    // ByteReader 接口实现
    int Read(char* dest, int start, int length) override;
    int ReadByte() override;
    int ReadInt() override;
    long long ReadLong() override;
    double ReadDouble() override;
    float ReadFloat() override;
    bool ReadBool() override;
    short ReadShort() override;
    std::string ReadString() override;
    bool Available() override;

private:
    std::string data_;  // 文件的全部字节
    int next_ = 0;      // 当前读取位置
};

}  // namespace common

#endif  //RTFS2D_FILE_BYTE_READER_H