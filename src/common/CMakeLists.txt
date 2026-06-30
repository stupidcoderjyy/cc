set(TARGET cc_common)

add_library(${TARGET} STATIC
        io/abstract_input.cpp
        io/buffered_input.cpp
        io/string_byte_reader.cpp
        io/string_input.cpp
        compile/compiler_input.cpp
        compile/compile_error.cpp
        compile/lexer.cpp
)
add_library(cc::common ALIAS ${TARGET})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
