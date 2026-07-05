//
// Created by PC on 2026/7/3.
//

#include "generator.h"

#include <inja/inja.hpp>
#include <iostream>

#include "compile/compiler_input.h"
#include "lex/dfa_builder.h"
#include "lex/nfa_regex_parser.h"
#include "script_parser_data.h"
#include "syntax/lalr_parser.h"
#include "util/print_util.h"

namespace cc::gen {

Generator::Generator(std::string name, std::string s_namespace) {
    regex_parser_ = std::make_unique<NFARegexParser>();
    dfa_builder_ = std::make_unique<DFABuilder>(*regex_parser_);
    syntax_ = std::make_unique<Syntax>();
    lalr_builder_ = std::make_unique<LALRBuilder>(*syntax_);
    info_ = std::make_unique<GenerateInfo>();
    info_->parser_name = std::move(name);
    info_->date = CurrentTimeString();
    info_->s_namespace = std::move(s_namespace);
    info_->lb = '\n';
}

void Generator::Build(const std::string& script_file, const std::string& dest_dir) {
    try {
        ScriptParserData spd(this, this, print_debug_info_);
        ScriptLexerData sld;
        common::Parser parser(spd);
        common::Lexer lexer(sld);
        auto ci = CompilerInput::FromFile(script_file);
        parser.Parse(lexer, *ci);
        auto json = BuildJson();
        RenderTemplate("templates/parser_data.h.txt", json,
                std::format("{}/{}_parser_data.h", dest_dir, info_->parser_name));
        RenderTemplate("templates/parser_data.cpp.txt", json,
                std::format("{}/{}_parser_data.cpp", dest_dir, info_->parser_name));
    } catch (common::CompileError& e) {
        std::cerr << e.FormatErrorMessage() << std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void Generator::DFASetCharClassCount(int count) {
    info_->lexer.char_class_count = count;
}

void Generator::DFASetStatesCount(int count) {
    info_->lexer.states_count = count;
}

void Generator::DFASetStartState(int id) {
    info_->lexer.start_state = id;
}

void Generator::DFASetCharToClass(int ch, int class_id) {
    info_->lexer.char_class_setters.emplace_back(ch, class_id);
}

void Generator::DFASetStateInfo(int state, bool accepted, const std::string& token) {
    if (!accepted) {
        return;
    }
    info_->lexer.accepted[state] = accepted;
    info_->lexer.state_to_token[state] = token;
    if (token != "single" && !info_->lexer.token_to_type.contains(token)) {
        info_->lexer.token_to_type[token] =
                kMaxChars + static_cast<int>(info_->lexer.token_to_type.size());
    }
}

void Generator::DFASetGoto(int start, int input, int target) {
    info_->lexer.goto_setters.emplace_back(start, input, target);
}

void Generator::DFAFinish() {}

Generator::~Generator() = default;

void Generator::LALRSetStatesCount(int count) {
    info_->parser.states = count;
}

void Generator::LALRSetProductions(const std::vector<Production>& prods) {
    info_->parser.productions.reserve(prods.size());
    for (const auto& prod : prods) {
        ProductionInfo pi;
        pi.full_text = prod.ToString();
        pi.head = prod.head.name;
        pi.body.reserve(prod.body.size());
        for (const auto& s : prod.body) {
            pi.body.push_back(s.name);
        }
        info_->parser.productions.push_back(std::move(pi));
    }
}

void Generator::LALRSetTerminals(const std::vector<Symbol>& symbols) {
    auto& parser = info_->parser;
    parser.terminals.reserve(symbols.size());
    int id = 1;
    for (const auto& s : symbols) {
        parser.terminals.push_back(s.name);
        parser.symbol_info_map[s.name] = {id, true};
        auto token_str = s.name.starts_with('\'') ? s.name : "kToken" + ToCamelCase(s.name);
        parser.token_mapper_setters.emplace_back(token_str, id);
        ++id;
    }
    parser.token_mappers = kMaxChars + static_cast<int>(info_->lexer.token_to_type.size());
}

void Generator::LALRSetNonTerminals(const std::vector<Symbol>& symbols) {
    auto& parser = info_->parser;
    parser.non_terminals.reserve(symbols.size());
    int id = 0;
    for (const auto& s : symbols) {
        parser.non_terminals.push_back(s.name);
        parser.symbol_info_map[s.name] = {id++, false};
    }
}

void Generator::LALRSetAction(int stateId, int symbolId, int type, int target) {
    info_->parser.actions_setter.emplace_back(stateId, symbolId, type, target);
}

void Generator::LALRSetGoto(int stateId, int symbolId, int target) {
    info_->parser.goto_setter.emplace_back(stateId, symbolId, target);
}

void Generator::LALRFinish() {}

void Generator::RenderTemplate(
        const std::string& template_file, const json& json, const std::string& dst_file) {
    // 1. 创建 Inja 环境
    inja::Environment env;
    using std::filesystem::create_directories;
    using std::filesystem::exists;
    using std::filesystem::path;
    using std::filesystem::current_path;

    // 2. 从模板文件读取并渲染，返回生成的字符串
    if (!exists(template_file)) {
        throw std::runtime_error(std::format("missing template file: {}/{}", current_path().string(), template_file));
    }
    std::string rendered = env.render_file(template_file, json);

    // 3. 确保输出目录存在
    if (path outDir = path(dst_file).parent_path(); !outDir.empty() && !exists(outDir)) {
        create_directories(outDir);
    }

    // 4. 写入输出文件
    std::ofstream outFile(dst_file);
    if (!outFile.is_open()) {
        throw std::runtime_error("Cannot open file " + dst_file);
    }
    outFile << rendered;
}

json Generator::BuildJson() const {
    return {
            {"date", info_->date},
            {"s_namespace", info_->s_namespace},
            {"parser_name_upper", ToUpperCase(info_->parser_name)},
            {"parser_name_camel", ToCamelCase(info_->parser_name)},
            {"parser_name_lower", info_->parser_name},
            {"lexer", BuildLexerJson()},
            {"parser", BuildParserJson()},
    };
}

json Generator::BuildLexerJson() const {
    auto& [char_class_count, states_count, start_state, accepted, state_to_token, goto_setters,
            char_class_setters, token_to_type] = info_->lexer;

    // 词法单元名称
    auto arr_tokens = json::array();
    for (const auto& [token, type] : token_to_type) {
        arr_tokens.push_back({
                {"name_camel", ToCamelCase(token)},
                {"type", type},
        });
    }

    // accepted数组
    auto arr_accepted = json::array();
    for (const auto& [i, v] : accepted) {
        arr_accepted.push_back({
                {"i", i},
                {"v", v},
        });
    }

    // goto数组
    auto arr_goto = json::array();
    for (const auto& [i1, i2, v] : goto_setters) {
        arr_goto.push_back({
                {"i1", i1},
                {"i2", i2},
                {"v", v},
        });
    }

    // 字符映射数组
    auto arr_char_to_class = json::array();
    for (const auto& [i, v] : char_class_setters) {
        arr_char_to_class.push_back({
                {"i", common::print::DisplayChar(i)},
                {"v", v},
        });
    }

    // 词法单元构造器数组
    auto arr_tokens_suppliers = json::array();
    for (const auto& [state, token] : state_to_token) {
        arr_tokens_suppliers.push_back({
                {"i", state},
                {"v", ToCamelCase(token)},
        });
    }

    return {
            {"tokens", std::move(arr_tokens)},
            {"char_class_count", char_class_count},
            {"states_count", states_count},
            {"start_state", start_state},
            {"arr_goto", std::move(arr_goto)},
            {"arr_char_to_class", std::move(arr_char_to_class)},
            {"arr_accepted", std::move(arr_accepted)},
            {"arr_token_suppliers", std::move(arr_tokens_suppliers)},
    };
}

json Generator::BuildParserJson() const {
    auto& parser = info_->parser;
    auto arr_productions = json::array();
    std::unordered_map<std::string, std::tuple<int, int>> head_to_prod_count;
    for (const auto& p : parser.productions) {
        auto& [a, b] = head_to_prod_count[p.head];
        b = ++a;
    }
    for (int i = 0; i < parser.productions.size(); ++i) {
        const auto& p = parser.productions[i];
        auto camel = ToCamelCase(p.head);
        auto camel_numbered = camel;
        if (auto& [a, b] = head_to_prod_count[p.head]; b > 1) {
            camel_numbered += std::to_string(b - a);
            --a;
        }
        arr_productions.push_back({
                {"full_text", p.full_text},
                {"head_camel", camel},
                {"head_camel_numbered", camel_numbered},
                {"prod_init_expr", FormatProductionInitExpr(i, p)},
        });
    }

    // goto数组
    auto arr_goto = json::array();
    for (const auto& [i1, i2, v] : parser.goto_setter) {
        arr_goto.push_back({
                {"i1", i1},
                {"i2", i2},
                {"v", v},
        });
    }

    // actions
    auto arr_actions = json::array();
    for (const auto& [i1, i2, t, v] : parser.actions_setter) {
        std::string v_str;
        if (t == 1)
            v_str = "kActionShift";
        else if (t == 2)
            v_str = "kActionReduce";
        else if (t == 3)
            v_str = "kActionAccept";
        else
            v_str = "0 //Undefined Action";
        if (t == 1 || t == 2) {
            v_str += " | " + std::to_string(v);
        }
        arr_actions.push_back({
                {"i1", i1},
                {"i2", i2},
                {"v", v_str},
        });
    }

    // token_to_symbol
    auto arr_token_to_symbol = json::array();
    for (const auto& [s, v] : parser.token_mapper_setters) {
        arr_token_to_symbol.push_back({
                {"i", s},
                {"v", v},
        });
    }
    return {
            {"arr_productions", std::move(arr_productions)},
            {"arr_goto", std::move(arr_goto)},
            {"arr_actions", std::move(arr_actions)},
            {"arr_token_to_symbol", std::move(arr_token_to_symbol)},
            {"non_terminal_count", info_->parser.non_terminals.size()},
            {"terminal_count", info_->parser.terminals.size() + 1},
            {"states_count", info_->parser.states},
            {"productions_count", info_->parser.productions.size()},
            {"token_mappers_count", info_->parser.token_mappers},
    };
}

std::string Generator::ToCamelCase(const std::string& text) {
    std::ostringstream oss;
    bool capitalizeNext = true;  // 首字母大写

    for (char ch : text) {
        if (ch == '_') {
            capitalizeNext = true;
        } else {
            if (capitalizeNext) {
                oss << static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                capitalizeNext = false;
            } else {
                oss << static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));  // 原样保留小写字母或数字
            }
        }
    }
    return oss.str();
}

std::string Generator::ToUpperCase(const std::string& text) {
    std::string result = text;
    std::ranges::transform(
            result, result.begin(), [](unsigned char ch) { return std::toupper(ch); });
    return result;
}

std::string Generator::CurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &time_t_now);
#else
    localtime_r(&time_t_now, &local_tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M");
    return oss.str();
}

std::string Generator::FormatSymbolInitExpr(const std::string& s) const {
    const auto& [id, accepted] = info_->parser.symbol_info_map[s];
    return std::format("{{ {}, {} }}", accepted, id);
}

std::string Generator::FormatProductionInitExpr(int id, const ProductionInfo& pi) const {
    std::ostringstream oss;
    oss << "{ " << id << ", " << FormatSymbolInitExpr(pi.head) << ", {";
    for (const auto& body : pi.body) {
        oss << FormatSymbolInitExpr(body) << ", ";
    }
    oss << "}}";
    return oss.str();
}
}  // namespace cc::gen
