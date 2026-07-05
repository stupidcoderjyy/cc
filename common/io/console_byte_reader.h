//
// Created by PC on 2026/7/6.
//

#ifndef CC_CONSOLE_BYTE_READER_H
#define CC_CONSOLE_BYTE_READER_H
#include "stream_byte_reader.h"

namespace common {

class ConsoleByteReader : public StreamByteReader {
public:
    ConsoleByteReader();
    int Read(char *dest, int start, int length) override;
};
}



#endif //CC_CONSOLE_BYTE_READER_H
