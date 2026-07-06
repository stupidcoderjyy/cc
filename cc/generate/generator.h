//
// Created by PC on 2026/7/3.
//

#ifndef CC_GENERATE_H
#define CC_GENERATE_H

#include <memory>
#include <nlohmann/json.hpp>

#include "lex/dfa_setter.h"
#include "syntax/language_setter.h"

namespace cc {
class Syntax;
class NFARegexParser;

class DFABuilder;
class LALRBuilder;

namespace gen {

struct ProductionInfo {
    std::string full_text;  // 产生式完整字符串
    std::string head;
    std::vector<std::string> body;
};

struct LexerInfo {
    int char_class_count;
    int states_count;
    int start_state;
    std::unordered_map<int, bool> accepted;
    std::unordered_map<int, std::string> state_to_token;
    std::vector<std::tuple<int, int, int>> goto_setters;
    std::vector<std::tuple<int, int>> char_class_setters;
    std::unordered_map<std::string, int> token_to_type;
};

struct ParserInfo {
    int token_mappers;
    int states;
    std::vector<std::string> non_terminals;
    std::vector<std::string> terminals;
    std::vector<std::tuple<std::string, int>> token_mapper_setters;
    std::vector<std::tuple<int, int, int, int>> actions_setter;
    std::vector<std::tuple<int, int, int>> goto_setter;
    std::vector<ProductionInfo> productions;
    std::unordered_map<std::string, std::tuple<int, bool>> symbol_info_map;
};

struct GenerateInfo {
    std::string s_namespace;
    std::string parser_name;
    std::string date;
    LexerInfo lexer;
    ParserInfo parser;
    std::string lb;
};

using nlohmann::json;

class Generator : public DFASetter, public LanguageSetter {
public:
    explicit Generator(std::string name, std::string s_namespace);
    void Build(const std::string& script_file, const std::string& dest_dir);

    // DFASetter
    void DFASetCharClassCount(int count) override;
    void DFASetStatesCount(int count) override;
    void DFASetStartState(int id) override;
    void DFASetCharToClass(int ch, int class_id) override;
    void DFASetStateInfo(int state, bool accepted, const std::string& token) override;
    void DFASetGoto(int start, int input, int target) override;
    void DFAFinish() override;
    ~Generator() override;

    // LanguageSetter
    void LALRSetStatesCount(int count) override;
    void LALRSetProductions(const std::vector<Production>& prods) override;
    void LALRSetTerminals(const std::vector<Symbol>& symbols) override;
    void LALRSetNonTerminals(const std::vector<Symbol>& non_terms) override;
    void LALRSetAction(int stateId, int symbolId, int type, int target) override;
    void LALRSetGoto(int stateId, int symbolId, int target) override;
    void LALRFinish() override;

    // setter
    void set_print_debug_info(bool enabled) { print_debug_info_ = enabled; }
    void set_warn_conflict(bool enabled) { warn_conflict_ = enabled; }

private:
    std::unique_ptr<Syntax> syntax_;
    std::unique_ptr<NFARegexParser> regex_parser_;
    std::unique_ptr<DFABuilder> dfa_builder_;
    std::unique_ptr<LALRBuilder> lalr_builder_;
    std::unique_ptr<GenerateInfo> info_;
    bool print_debug_info_ = false;
    bool warn_conflict_ = true;

    static void RenderTemplate(
            const std::string_view& file_str, const json& json, const std::string& dst_file);
    json BuildJson() const;
    json BuildLexerJson() const;
    json BuildParserJson() const;
    static std::string ToCamelCase(const std::string& text);
    static std::string ToUpperCase(const std::string& text);
    static std::string CurrentTimeString();
    std::string FormatSymbolInitExpr(const std::string& s) const;
    std::string FormatProductionInitExpr(const ProductionInfo &pi) const;
};

}  // namespace gen

}  // namespace cc

#endif  //CC_GENERATE_H
