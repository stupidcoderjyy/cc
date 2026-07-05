//
// Created by PC on 2026/7/6.
//

#include "io/stream_byte_reader.h"

#include <gtest/gtest.h>

#include <cstring>
#include <stdexcept>
#include <vector>

#include "io/byte_reader.h"

namespace common {
namespace {

// Helper: create reader from raw bytes
std::unique_ptr<StreamByteReader> MakeReader(const std::vector<char>& bytes) {
    return StreamByteReader::FromString(std::string(bytes.data(), bytes.size()));
}

// ==================== FromString ====================

TEST(StreamByteReaderTest, FromStringBasic) {
    auto r = StreamByteReader::FromString("hello");
    EXPECT_EQ(r->ReadByte(), 'h');
    EXPECT_EQ(r->ReadByte(), 'e');
}

TEST(StreamByteReaderTest, FromStringEmpty) {
    auto r = StreamByteReader::FromString("");
    EXPECT_FALSE(r->Available());
}

// ==================== ReadByte ====================

TEST(StreamByteReaderTest, ReadByteSequence) {
    auto r = MakeReader({'A', 'B', 'C'});
    EXPECT_EQ(r->ReadByte(), 'A');
    EXPECT_EQ(r->ReadByte(), 'B');
    EXPECT_EQ(r->ReadByte(), 'C');
}

TEST(StreamByteReaderTest, ReadByteThrowsOnEOF) {
    auto r = MakeReader({'X'});
    r->ReadByte();
    EXPECT_THROW(r->ReadByte(), std::runtime_error);
}

// ==================== ReadInt (4 bytes, little-endian) ====================

TEST(StreamByteReaderTest, ReadInt) {
    // 0x01020304 in little-endian
    auto r = MakeReader({'\x04', '\x03', '\x02', '\x01'});
    EXPECT_EQ(r->ReadInt(), 0x01020304);
}

TEST(StreamByteReaderTest, ReadIntNegative) {
    // -1 in little-endian = 0xFFFFFFFF
    auto r = MakeReader({'\xFF', '\xFF', '\xFF', '\xFF'});
    EXPECT_EQ(r->ReadInt(), -1);
}

TEST(StreamByteReaderTest, ReadIntIncomplete) {
    auto r = MakeReader({'\x01', '\x02'});
    EXPECT_THROW(r->ReadInt(), std::runtime_error);
}

// ==================== ReadShort (2 bytes, little-endian) ====================

TEST(StreamByteReaderTest, ReadShort) {
    auto r = MakeReader({'\x34', '\x12'});
    EXPECT_EQ(r->ReadShort(), 0x1234);
}

TEST(StreamByteReaderTest, ReadShortNegative) {
    auto r = MakeReader({'\xFF', '\xFF'});
    EXPECT_EQ(r->ReadShort(), -1);
}

TEST(StreamByteReaderTest, ReadShortIncomplete) {
    auto r = MakeReader({'\x01'});
    EXPECT_THROW(r->ReadShort(), std::runtime_error);
}

// ==================== ReadLong (8 bytes, little-endian) ====================

TEST(StreamByteReaderTest, ReadLong) {
    std::vector<char> bytes(8);
    std::int64_t val = 0x0102030405060708LL;
    std::memcpy(bytes.data(), &val, 8);
    auto r = MakeReader(bytes);
    EXPECT_EQ(r->ReadLong(), val);
}

TEST(StreamByteReaderTest, ReadLongIncomplete) {
    auto r = MakeReader({'\x01', '\x02', '\x03', '\x04'});
    EXPECT_THROW(r->ReadLong(), std::runtime_error);
}

// ==================== ReadBool ====================

TEST(StreamByteReaderTest, ReadBoolTrue) {
    auto r = MakeReader({'\x01'});
    EXPECT_TRUE(r->ReadBool());
}

TEST(StreamByteReaderTest, ReadBoolFalse) {
    auto r = MakeReader({'\x00'});
    EXPECT_FALSE(r->ReadBool());
}

TEST(StreamByteReaderTest, ReadBoolNonZeroIsTrue) {
    auto r = MakeReader({'\xFF'});
    EXPECT_TRUE(r->ReadBool());
}

TEST(StreamByteReaderTest, ReadBoolThrowsOnEOF) {
    auto r = MakeReader({});
    EXPECT_THROW(r->ReadBool(), std::runtime_error);
}

// ==================== ReadFloat ====================

TEST(StreamByteReaderTest, ReadFloat) {
    float val = 3.14f;
    std::vector<char> bytes(sizeof(float));
    std::memcpy(bytes.data(), &val, sizeof(float));
    auto r = MakeReader(bytes);
    EXPECT_FLOAT_EQ(r->ReadFloat(), val);
}

TEST(StreamByteReaderTest, ReadFloatIncomplete) {
    auto r = MakeReader({'\x01', '\x02'});
    EXPECT_THROW(r->ReadFloat(), std::runtime_error);
}

// ==================== ReadDouble ====================

TEST(StreamByteReaderTest, ReadDouble) {
    double val = 2.718281828;
    std::vector<char> bytes(sizeof(double));
    std::memcpy(bytes.data(), &val, sizeof(double));
    auto r = MakeReader(bytes);
    EXPECT_DOUBLE_EQ(r->ReadDouble(), val);
}

TEST(StreamByteReaderTest, ReadDoubleIncomplete) {
    auto r = MakeReader({'\x01', '\x02', '\x03', '\x04'});
    EXPECT_THROW(r->ReadDouble(), std::runtime_error);
}

// ==================== ReadString ====================

TEST(StreamByteReaderTest, ReadString) {
    // length=5 (0x05,0,0,0) then "world"
    auto r = MakeReader({'\x05', '\x00', '\x00', '\x00', 'w', 'o', 'r', 'l', 'd'});
    EXPECT_EQ(r->ReadString(), "world");
}

TEST(StreamByteReaderTest, ReadStringEmpty) {
    // length=0
    auto r = MakeReader({'\x00', '\x00', '\x00', '\x00'});
    EXPECT_EQ(r->ReadString(), "");
}

TEST(StreamByteReaderTest, ReadStringNegative) {
    // length=-1
    auto r = MakeReader({'\xFF', '\xFF', '\xFF', '\xFF'});
    EXPECT_THROW(r->ReadString(), std::runtime_error);
}

TEST(StreamByteReaderTest, ReadStringTruncatedData) {
    // length=10, but only 2 bytes follow
    auto r = MakeReader({'\x0A', '\x00', '\x00', '\x00', 'a', 'b'});
    EXPECT_THROW(r->ReadString(), std::runtime_error);
}

TEST(StreamByteReaderTest, ReadStringTruncatedLength) {
    auto r = MakeReader({'\x05', '\x00'});
    EXPECT_THROW(r->ReadString(), std::runtime_error);
}

// ==================== Raw Read ====================

TEST(StreamByteReaderTest, RawRead) {
    auto r = MakeReader({'H', 'e', 'l', 'l', 'o'});
    char buf[10]{};
    int n = r->Read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(std::string(buf, 3), "Hel");
    n = r->Read(buf, 3, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(std::string(buf, 5), "Hello");
}

TEST(StreamByteReaderTest, RawReadPastEnd) {
    auto r = MakeReader({'A', 'B'});
    char buf[10]{};
    int n = r->Read(buf, 0, 5);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(buf[0], 'A');
    EXPECT_EQ(buf[1], 'B');
}

TEST(StreamByteReaderTest, RawReadNullDest) {
    auto r = MakeReader({'A'});
    EXPECT_EQ(r->Read(nullptr, 0, 1), 0);
}

TEST(StreamByteReaderTest, RawReadNegativeStart) {
    auto r = MakeReader({'A'});
    EXPECT_EQ(r->Read(nullptr, -1, 1), 0);
}

TEST(StreamByteReaderTest, RawReadZeroLength) {
    auto r = MakeReader({'A'});
    char buf[10]{};
    EXPECT_EQ(r->Read(buf, 0, 0), 0);
}

// ==================== Available ====================

TEST(StreamByteReaderTest, AvailableTrue) {
    auto r = MakeReader({'X'});
    EXPECT_TRUE(r->Available());
}

TEST(StreamByteReaderTest, AvailableAfterRead) {
    auto r = MakeReader({'X', 'Y'});
    EXPECT_TRUE(r->Available());
    r->ReadByte();
    EXPECT_TRUE(r->Available());
    r->ReadByte();
    EXPECT_FALSE(r->Available());
}

TEST(StreamByteReaderTest, AvailableEmpty) {
    auto r = MakeReader({});
    EXPECT_FALSE(r->Available());
}

// ==================== FromFile ====================

TEST(StreamByteReaderTest, FromFileNotFound) {
    EXPECT_THROW(
            StreamByteReader::FromFile("/nonexistent/file/stream_test.bin"), std::runtime_error);
}

// ==================== Ownership model (unique_ptr istream) ====================

TEST(StreamByteReaderTest, OwnedStream) {
    // Test that the unique_ptr-owning constructor works
    auto iss = std::make_unique<std::istringstream>("owned");
    StreamByteReader r(std::move(iss));
    EXPECT_TRUE(r.Available());
    EXPECT_EQ(r.ReadByte(), 'o');
}

TEST(StreamByteReaderTest, ReferenceStream) {
    std::istringstream iss("ref");
    StreamByteReader r(iss);
    EXPECT_TRUE(r.Available());
    EXPECT_EQ(r.ReadByte(), 'r');
}

}  // namespace
}  // namespace common
