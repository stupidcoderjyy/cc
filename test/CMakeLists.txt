enable_testing()
include(GoogleTest)

set(TARGET cc_test)

add_executable(${TARGET}
        lex/dfa_builder_build_char_class_map_test.cpp
        lex/dfa_builder_epsilon_closure_test.cpp
        lex/dfa_builder_next_test.cpp
        lex/dfa_builder_build_dfa_test.cpp
        lex/dfa_builder_minimize_dfa_test.cpp
        lex/dfa_builder_test.cpp
        lex/nfa_regex_parser_test.cpp
        syntax/syntax_test.cpp
        syntax/lalr_parser_compute_first_test.cpp
        syntax/lalr_parser_lr0_test.cpp
        syntax/lalr_parser_canonical_test.cpp
        syntax/lalr_parser_follow_test.cpp
        syntax/lalr_parser_lalr_merge_test.cpp
        syntax/lalr_parser_lookahead_test.cpp
        syntax/lalr_parser_parsing_table_test.cpp
)

target_link_libraries(${TARGET} PRIVATE
        cc::core
        GTest::gtest_main
)

gtest_discover_tests(${TARGET})

add_dependencies(${TARGET} format)
