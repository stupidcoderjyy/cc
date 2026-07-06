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
        generate/script_parser_data.cpp
        generate/generator.cpp
        generate/embedded_templates.cpp
)
target_link_libraries(${TARGET} PRIVATE
        stupidcoder::common
        pantor::inja
        nlohmann_json::nlohmann_json
)
add_library(stupidcoder::cc_core ALIAS ${TARGET})
target_include_directories(cc_core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/stupidcoder/cc>  # 安装后的相对路径
)