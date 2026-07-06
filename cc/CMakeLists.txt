set(TARGET stupidcoder.cc_core)

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
        generate/script_parser_data.cpp
        generate/generator.cpp
)
target_link_libraries(${TARGET} PRIVATE
        jyy::common
        jyy::cc_templates
        pantor::inja
        nlohmann_json::nlohmann_json
)
add_library(jyy::cc_core ALIAS ${TARGET})
target_include_directories(${TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)