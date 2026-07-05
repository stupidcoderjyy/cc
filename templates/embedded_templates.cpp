//
// Created by PC on 2026/7/6.
//

#include "embedded_templates.h"

namespace cc::embed {

std::string_view GetParserDataHeaderTemplate() {
    return R"(//
// Generated Parser file by stupidcoder cc
// Date: {{ date }}
//

#ifndef CC_{{ parser_name_upper }}_PARSER_DATA_H
#define CC_{{ parser_name_upper }}_PARSER_DATA_H

#include "compile/lexer.h"
#include "compile/parser.h"

namespace cc {

struct DFASetter;
struct LanguageSetter;

}  // namespace cc

namespace {{ s_namespace }} {

// ==================== Token Types ====================

## for item in lexer.tokens
constexpr int kToken{{ item.name_camel }} = {{ item.type }};
## endfor

// ==================== Lexer Data ====================

struct {{ parser_name_camel }}LexerData : common::LexerDataSupplier {
    int CharClassCount() override;
    int StatesCount() override;
    int StartState() override;
    void InitAccepted(std::vector<bool>& vec) override;
    void InitGoto(std::vector<std::vector<int>>& vec) override;
    void InitCharToClass(std::vector<int>& vec) override;
    void InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) override;
};

// ==================== Token Structs ====================

using common::CompilerInput;
using common::Token;
using common::TokenMatchResult;
using common::TokenSingle;

{% for item in lexer.tokens %}struct Token{{ item.name_camel }} : Token {
    int Type() override;
};{% endfor %}

// ==================== Property ====================

using common::Property;
using common::PropertyTerminal;

// ==================== Parser Data ====================

class {{ parser_name_camel }}ParserData : public common::ParserDataSupplier {
public:
    int TokenMappersCount() const override;
    int NonTerminalSymbolsCount() const override;
    int TerminalSymbolsCount() const override;
    int StatesCount() const override;
    int ProductionsCount() const override;
    void InitReduceActions(std::vector<common::ReduceFunc>& vec) override;
    void InitGoto(std::vector<std::vector<int>>& vec) override;
    void InitActions(std::vector<std::vector<int>>& vec) override;
    void InitTokenToSymbol(std::vector<int>& vec) override;
    void InitProductions(std::vector<common::Production>& productions) override;

protected:
{% for item in parser.arr_productions %}    // {{ item.full_text }}
    virtual void Reduce{{ item.head_camel_numbered }}(const std::vector<std::unique_ptr<Property>>& props);
{% endfor -%}
};

}  // namespace {{ s_namespace }}

#endif  //CC_{{ parser_name_upper }}_PARSER_DATA_H)";
}

std::string_view GetParserDataCppTemplate() {
return R"(//
// Generated Parser file by stupidcoder cc
// Date: {{ date }}
//

#include "{{ parser_name_lower }}_parser_data.h"

namespace {{ s_namespace }} {

// ==================== Token Structs Impl ====================

{% for item in lexer.tokens %}int Token{{ item.name_camel }}::Type() {
    return kToken{{ item.name_camel }};
}
{% endfor -%}

// ==================== Lexer Data ====================

int {{ parser_name_camel }}LexerData::CharClassCount() {
    return {{ lexer.char_class_count }};
}

int {{ parser_name_camel }}LexerData::StatesCount() {
    return {{ lexer.states_count }};
}

int {{ parser_name_camel }}LexerData::StartState() {
    return {{ lexer.start_state }};
}

void {{ parser_name_camel }}LexerData::InitAccepted(std::vector<bool>& vec) {
{% for item in lexer.arr_accepted %}    vec[{{ item.i }}] = {{ item.v }};
{% endfor -%}
}

void {{ parser_name_camel }}LexerData::InitGoto(std::vector<std::vector<int>>& vec) {
    // 三元组：{当前状态, 输入类, 目标状态}
    struct Trans {
        int s, c, t;
    };
    std::vector<Trans> trans = {
{% for item in lexer.arr_goto %}            { {{ item.i1 }}, {{ item.i2 }}, {{ item.v }} },
{% endfor %}    };
    for (const auto& t : trans) {
        vec[t.s][t.c] = t.t;
    }
}

void {{ parser_name_camel }}LexerData::InitCharToClass(std::vector<int>& vec) {
{% for item in lexer.arr_char_to_class %}    vec[{{ item.i }}] = {{ item.v }};
{% endfor -%}
}

void {{ parser_name_camel }}LexerData::InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) {
{% for item in lexer.arr_token_suppliers %}    vec[{{ item.i }}] = [] { return std::make_unique<Token{{ item.v }}>(); };
{% endfor -%}
}

// ==================== Parser Data ====================

int {{ parser_name_camel }}ParserData::TokenMappersCount() const {
    return {{ parser.token_mappers_count }};
}
int {{ parser_name_camel }}ParserData::NonTerminalSymbolsCount() const {
    return {{ parser.non_terminal_count }};
}
int {{ parser_name_camel }}ParserData::TerminalSymbolsCount() const {
    return {{ parser.terminal_count }};
}
int {{ parser_name_camel }}ParserData::StatesCount() const {
    return {{ parser.states_count }};
}
int {{ parser_name_camel }}ParserData::ProductionsCount() const {
    return {{ parser.productions_count }};
}

void {{ parser_name_camel }}ParserData::InitReduceActions(std::vector<common::ReduceFunc>& vec) {
{% for item in parser.arr_productions %}    vec[{{ loop.index }}] = [this](auto& p) { Reduce{{ item.head_camel_numbered }}(p); };  // {{ item.full_text }}
{% endfor -%}
}

void {{ parser_name_camel }}ParserData::InitGoto(std::vector<std::vector<int>>& vec) {
{% for item in parser.arr_goto %}    vec[{{ item.i1 }}][{{ item.i2 }}] = {{ item.v }};
{% endfor -%}
}

void {{ parser_name_camel }}ParserData::InitActions(std::vector<std::vector<int>>& vec) {
    using common::kActionAccept;
    using common::kActionReduce;
    using common::kActionShift;
{% for item in parser.arr_actions %}    vec[{{ item.i1 }}][{{ item.i2 }}] = {{ item.v }};
{% endfor -%}
}

void {{ parser_name_camel }}ParserData::InitTokenToSymbol(std::vector<int>& vec) {
{% for item in parser.arr_token_to_symbol %}    vec[{{ item.i }}] = {{ item.v }};
{% endfor -%}
}

void {{ parser_name_camel }}ParserData::InitProductions(std::vector<common::Production>& prods) {
    // Productions
{% for item in parser.arr_productions %}    prods[{{ loop.index }}] = {{ item.prod_init_expr }}; // {{ item.full_text }}
{% endfor -%}
}

{% for item in parser.arr_productions %}// {{ item.full_text }}
void {{ parser_name_camel }}ParserData::Reduce{{ item.head_camel_numbered }}(const std::vector<std::unique_ptr<Property>>& props){}
{% endfor %}
}  // namespace {{ s_namespace }})";
}

}
