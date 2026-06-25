set(TARGET cc_core)

add_library(${TARGET} STATIC
        io/abstract_input.cpp
        io/buffered_input.cpp
        io/string_byte_reader.cpp
        io/string_input.cpp
        lex/nfa_node.cpp
        lex/nfa.cpp
        lex/nfa_regex_parser.cpp
)
add_library(cc::core ALIAS ${TARGET})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})