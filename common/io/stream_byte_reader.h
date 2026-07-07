//
// Created by PC on 2026/7/6.
//

#ifndef CC_STREAM_BYTE_READER_H
#define CC_STREAM_BYTE_READER_H

#include <istream>
#include <memory>

#include "byte_reader.h"

namespace common {

class StreamByteReader : public ByteReader {
public:
    // 绑定到输入流（必须确保流在对象生命周期内有效）
    explicit StreamByteReader(std::istream& stream);
    explicit StreamByteReader(std::unique_ptr<std::istream> stream);
    static std::unique_ptr<StreamByteReader> FromString(const std::string& str);
    static std::unique_ptr<StreamByteReader> FromFile(const std::string& path);
    static std::unique_ptr<StreamByteReader> FromConsole();

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
    ~StreamByteReader() override = default;

protected:
    std::istream& stream_;

private:
    std::unique_ptr<std::istream> owned_stream_;
};

}  // namespace common

#endif  //CC_STREAM_BYTE_READER_H
