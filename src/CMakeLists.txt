add_subdirectory(common)

set(TARGET cc_core)

add_library(${TARGET} STATIC
        lex/nfa_node.cpp
        lex/nfa.cpp
        lex/nfa_regex_parser.cpp
        lex/dfa_builder.cpp
        lex/dfa_minimizer.cpp
        lex/dfa_setter.h
        util/bitset_hash.h
        syntax/syntax.cpp
        syntax/lalr_parser.cpp
        syntax/lalr_conflict_handler.cpp
)
target_link_libraries(${TARGET} PRIVATE
        cc::common
)
add_library(cc::core ALIAS ${TARGET})
target_include_directories(${TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)