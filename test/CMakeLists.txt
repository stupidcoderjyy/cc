enable_testing()
include(GoogleTest)

set(TEST_SRC_COMMON
        common/compile/lexer_test.cpp
        common/compile/parser_test.cpp
        common/compile/scriptloader_dump_test.cpp
        common/compile/script_loader_end_to_end_test.cpp
        common/io/stream_byte_reader_test.cpp
)
set(TEST_SRC_LEX
        lex/dfa_builder_build_char_class_map_test.cpp
        lex/dfa_builder_epsilon_closure_test.cpp
        lex/dfa_builder_next_test.cpp
        lex/dfa_builder_build_dfa_test.cpp
        lex/dfa_builder_minimize_dfa_test.cpp
        lex/dfa_builder_test.cpp
        lex/nfa_regex_parser_test.cpp
)
set(TEST_SRC_SYNTAX
        syntax/syntax_test.cpp
        syntax/lalr_parser_compute_first_test.cpp
        syntax/lalr_parser_lr0_test.cpp
        syntax/lalr_parser_canonical_test.cpp
        syntax/lalr_parser_follow_test.cpp
        syntax/lalr_parser_lookahead_test.cpp
        syntax/lalr_parser_parsing_table_test.cpp
        syntax/lalr_parser_parse_test.cpp
)
set(TEST_SRC_GENERATE
        generate/generator_test.cpp
)

add_executable(cc_test_common ${TEST_SRC_COMMON})
add_executable(cc_test_lex ${TEST_SRC_LEX})
add_executable(cc_test_syntax ${TEST_SRC_SYNTAX})
add_executable(cc_test_generate ${TEST_SRC_GENERATE})
add_executable(cc_test_all
        ${TEST_SRC_COMMON}
        ${TEST_SRC_LEX}
        ${TEST_SRC_SYNTAX}
        ${TEST_SRC_GENERATE}
)

set(TARGETS cc_test_common cc_test_lex cc_test_syntax cc_test_generate cc_test_all)
foreach(TARGET IN LISTS TARGETS)
    target_link_libraries(${TARGET} PRIVATE
            stupidcoder::cc_core
            stupidcoder::common
            GTest::gtest_main
            stdc++exp
    )
    gtest_discover_tests(${TARGET}
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
    add_dependencies(${TARGET} format prepare_test_files)
endforeach()

set(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/lang.txt)

add_custom_target(prepare_test_files
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/examples/calculator/lang.txt
            ${CMAKE_CURRENT_BINARY_DIR}/lang.txt
)
