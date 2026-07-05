set(TARGET stupidcoder.common)

add_library(${TARGET}
        io/abstract_input.cpp
        io/buffered_input.cpp
        io/string_input.cpp
        io/stream_byte_reader.cpp
        io/console_byte_reader.cpp
        compile/compiler_input.cpp
        compile/compile_error.cpp
        compile/lexer.cpp
        compile/parser.cpp
        util/print_util.cpp
        util/print_util.h
)
add_library(jyy::common ALIAS ${TARGET})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
