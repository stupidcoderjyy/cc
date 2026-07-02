//
// Created by PC on 2026/7/2.
//
// Debug test: register the scriptloader grammar and dump all LALR/DFA data.
// Intended for manual inspection — not an automated pass/fail test.
//

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <utility>

#include "cc_constants.h"
#include "compile/compiler_input.h"
#include "compile/lexer.h"
#include "lex/dfa_builder.h"
#include "lex/dfa_setter.h"
#include "lex/nfa_regex_parser.h"
#include "syntax/lalr_parser.h"
#include "syntax/language_setter.h"
#include "syntax/syntax.h"

namespace cc {
namespace {

std::string action_name(int type) {
    switch (type) {
        case 1:
            return "ACCEPT";
        case 2:
            return "SHIFT";
        case 3:
            return "REDUCE";
        default:
            return "???";
    }
}

// ==================== ConsoleLanguageSetter ====================

class ConsoleLanguageSetter : public LanguageSetter {
public:
    explicit ConsoleLanguageSetter(Syntax* s) : syntax_(s) {}

    void SetProductions(const std::vector<Production>& prods) override {
        std::cout << "====== PRODUCTIONS ======\n";
        for (const auto& p : prods) {
            std::cout << "  " << p.id << ": " << p.head.name;
            if (p.priority) std::cout << " (prio=" << p.priority << ")";
            std::cout << " ->";
            if (p.body.empty()) {
                std::cout << " ε";
            } else {
                for (const auto& s : p.body)
                    std::cout << " " << s.name;
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    void SetTerminals(const std::vector<Symbol>& terms) override {
        std::cout << "====== TERMINALS ======\n";
        for (const auto& s : terms) {
            std::cout << "  " << s.name;
            if (s.priority) std::cout << " prio=" << s.priority;
            if (s.assoc != Associativity::kLeft)
                std::cout << " assoc=" << (s.assoc == Associativity::kRight ? "RIGHT" : "NONASSOC");
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    void SetNonTerminals(const std::vector<Symbol>& non_terms) override {
        std::cout << "====== NON-TERMINALS ======\n";
        for (const auto& s : non_terms)
            std::cout << "  " << s.name << "\n";
        std::cout << std::endl;
    }

    void SetLALRState(int stateId, const std::set<Item>& items) override {
        if (static_cast<int>(state_items_.size()) <= stateId) state_items_.resize(stateId + 1);
        state_items_[stateId] = items;
    }

    void SetStateLookaheads(int stateId,
                            const std::map<Item, std::set<Symbol, std::less<>>>& la) override {
        if (static_cast<int>(state_lookahead_.size()) <= stateId)
            state_lookahead_.resize(stateId + 1);
        state_lookahead_[stateId] = la;
    }

    void SetAction(int stateId, int symbolId, int type, int target) override {
        std::cout << "  ACTION[" << stateId << "][" << symbolId << "] = " << action_name(type)
                  << " " << target << "\n";
    }

    void SetGoto(int stateId, int symbolId, int target) override {
        std::cout << "  GOTO[" << stateId << "][" << symbolId << "] = " << target << "\n";
    }

    void SetStartState(int /*id*/) override {}

    void Finish() override {
        std::cout << "\n====== LALR STATES (with lookaheads) ======\n";
        for (size_t s = 0; s < state_items_.size(); ++s) {
            std::cout << "State " << s << ":\n";
            for (const auto& item : state_items_[s]) {
                const auto& prod = syntax_->productions()[item.prod_id];
                std::cout << "  [" << item.prod_id << "] " << prod.head.name << " ->";
                for (int i = 0; i < static_cast<int>(prod.body.size()); ++i) {
                    if (i == item.dot_pos) std::cout << " ·";
                    std::cout << " " << prod.body[i].name;
                }
                if (item.dot_pos == static_cast<int>(prod.body.size())) std::cout << " ·";
                if (s < state_lookahead_.size()) {
                    auto lit = state_lookahead_[s].find(item);
                    if (lit != state_lookahead_[s].end() && !lit->second.empty()) {
                        std::cout << "   LA={";
                        bool first = true;
                        for (const auto& la : lit->second) {
                            if (!first) std::cout << ", ";
                            first = false;
                            std::cout << la.name;
                        }
                        std::cout << "}";
                    }
                }
                std::cout << "\n";
            }
        }
        std::cout << std::flush;
    }

private:
    Syntax* syntax_;
    std::vector<std::set<Item>> state_items_;
    std::vector<std::map<Item, std::set<Symbol, std::less<>>>> state_lookahead_;
};

// ==================== DFA collector for dump ====================

class DumpDFASetter : public DFASetter {
public:
    int class_count = 0;
    int state_count = 0;
    int start_state = 0;
    std::vector<int> char_to_class;
    std::vector<bool> accepted;
    std::vector<std::string> tokens;
    std::vector<std::map<int, int>> class_transitions;

    void SetCharClassCount(int c) override { class_count = c; }

    void SetCharToClass(int ch, int class_id) override {
        if (static_cast<size_t>(ch) >= char_to_class.size()) char_to_class.resize(ch + 1, 0);
        char_to_class[ch] = class_id;
    }

    void SetDfaStatesCount(int n) override {
        state_count = n;
        accepted.assign(n, false);
        tokens.resize(n);
        class_transitions.resize(n);
    }

    void SetStartState(int id) override { start_state = id; }

    auto SetStateInfo(int state, bool b, const std::string& token) -> void override {
        this->accepted[state] = b;
        if (b) tokens[state] = token;
    }

    void SetGoto(int start, int input, int target) override {
        class_transitions[start][input] = target;
    }

    void Finish() override {}
};

// ==================== Test Fixture ====================

class ScriptLoaderDumpTest : public ::testing::Test {
protected:
    void SetUp() override {
        RegisterTokens();
        RegisterGrammar();
    }

    void RegisterTokens() {
        auto& p = parser_;

        p.Register(R"(\a\w*)", "id");
        p.Register(R"(\"[^\"]*\")", "string");
        p.Register(R"(%%TOKEN)", "token_begin");
        p.Register(R"(%%SYNTAX)", "syntax_begin");
        p.Register(R"(%%)", "block_end");
        p.Register(R"(@($\a+|\a+|~)|'(.|\\.)')", "terminal");
        p.Register(R"(%\d*[rRlL])", "prod_mark");
        p.Register(R"($\d*[rRlL])", "symb_mark");

        parser_.RegisterSingles({':', ';', '|'});
    }

    void RegisterGrammar() {
        auto nt = [](const char* n) { return Symbol{n, SymbolType::kNonTerminal}; };
        auto t = [](const char* n) { return Symbol{n, SymbolType::kTerminal}; };
        auto eps = std::vector<Symbol>{};

        using I = std::initializer_list<Symbol>;

        // script : @token_begin tokens @block_end @syntax_begin syntax @block_end
        syntax_.AddProduction(nt("script"), I{t("token_begin"), nt("tokens"), t("block_end"),
                                              t("syntax_begin"), nt("syntax"), t("block_end")});
        // tokens : token | tokens token
        syntax_.AddProduction(nt("tokens"), I{nt("token")});
        syntax_.AddProduction(nt("tokens"), I{nt("tokens"), nt("token")});

        // token : @id ':' @string ';'
        syntax_.AddProduction(nt("token"), I{t("id"), t(":"), t("string"), t(";")});

        // syntax : prod | syntax prod
        syntax_.AddProduction(nt("syntax"), I{nt("prod")});
        syntax_.AddProduction(nt("syntax"), I{nt("syntax"), nt("prod")});

        // prod : @id ':' body ';'
        syntax_.AddProduction(nt("prod"), I{t("id"), t(":"), nt("body"), t(";")});

        // body : slice | body '|' slice
        syntax_.AddProduction(nt("body"), I{nt("slice")});
        syntax_.AddProduction(nt("body"), I{nt("body"), t("|"), nt("slice")});

        // slice : seq prod_priority
        syntax_.AddProduction(nt("slice"), I{nt("seq"), nt("prod_priority")});

        // prod_priority : @~ | @prod_priority_mark
        syntax_.AddProduction(nt("prod_priority"), eps);
        syntax_.AddProduction(nt("prod_priority"), I{t("prod_mark")});

        // seq : symbol | seq symbol
        syntax_.AddProduction(nt("seq"), I{nt("symbol")});
        syntax_.AddProduction(nt("seq"), I{nt("seq"), nt("symbol")});

        // symbol : @id | @terminal symb_priority
        syntax_.AddProduction(nt("symbol"), I{t("id")});
        syntax_.AddProduction(nt("symbol"), I{t("terminal"), nt("symb_priority")});

        // symb_priority : @~ | @symb_priority_mark
        syntax_.AddProduction(nt("symb_priority"), eps);
        syntax_.AddProduction(nt("symb_priority"), I{t("symb_mark")});
    }

    NFARegexParser parser_;
    Syntax syntax_;
};

// ==================== Dump token DFA ====================

TEST_F(ScriptLoaderDumpTest, DumpTokenDFA) {
    std::cout << "\n========== TOKEN DFA DATA ==========\n";

    DumpDFASetter dfa_dump;
    DFABuilder builder(parser_, &dfa_dump);

    std::cout << "Char classes: " << dfa_dump.class_count << "\n";
    std::cout << "DFA states: " << dfa_dump.state_count << "\n";
    std::cout << "DFA start: " << dfa_dump.start_state << "\n\n";

    for (int i = 0; i < dfa_dump.state_count; ++i) {
        std::cout << "State " << i;
        if (dfa_dump.accepted[i]) std::cout << " ACCEPT token=<<" << dfa_dump.tokens[i] << ">>";
        std::cout << "\n";
        for (const auto& [cid, target] : dfa_dump.class_transitions[i]) {
            std::cout << "  class " << cid << " → state " << target << "\n";
        }
    }

    std::cout << "\nChar → class (selected):\n";
    for (auto c : {'\000', ' ', '\n', '\t', 'a', 'z', 'A', 'Z', '0', '9', '_', ':', ';', '|', '%',
                   '$', '"', '\''}) {
        int cls = 0;
        if (c >= 0 && c < static_cast<int>(dfa_dump.char_to_class.size()))
            cls = dfa_dump.char_to_class[c];
        std::cout << "  '" << static_cast<char>(c) << "' (" << c << ") → class " << cls << "\n";
    }
    std::cout << std::flush;
}

// ==================== Dump grammar LALR ====================

TEST_F(ScriptLoaderDumpTest, DumpGrammarLALR) {
    std::cout << "\n========== GRAMMAR LALR DATA ==========\n";

    ConsoleLanguageSetter setter(&syntax_);
    LALRBuilder builder(syntax_);
    builder.set_print_conflict_info(true);
    builder.set_print_debug_info(true);
    builder.Build(&setter);

    // FIRST sets
    std::cout << "\n====== FIRST SETS ======\n";
    for (int i = 0; i < builder.SymbolCount(); ++i) {
        const auto& sym = builder.SymbolOf(i);
        if (sym.type != SymbolType::kNonTerminal) continue;
        std::cout << "FIRST(" << sym.name << ") = {";
        bool first = true;
        for (const auto& s : builder.First(i)) {
            if (!first) std::cout << ", ";
            first = false;
            std::cout << s.name;
        }
        if (builder.HasEpsilon(i)) {
            if (!first) std::cout << ", ";
            std::cout << "ε";
        }
        std::cout << "}\n";
    }

    // FOLLOW sets
    std::cout << "\n====== FOLLOW SETS ======\n";
    for (int i = 0; i < builder.SymbolCount(); ++i) {
        const auto& sym = builder.SymbolOf(i);
        if (sym.type != SymbolType::kNonTerminal) continue;
        const auto& f = builder.Follow(i);
        if (f.empty()) continue;
        std::cout << "FOLLOW(" << sym.name << ") = {";
        bool first = true;
        for (const auto& s : f) {
            if (!first) std::cout << ", ";
            first = false;
            std::cout << s.name;
        }
        std::cout << "}\n";
    }

    // Parse table compact view
    std::cout << "\n====== PARSE TABLE ======\n";
    std::cout << "  LR0 states: " << builder.lr0_states().size() << std::endl;

    for (size_t s = 0; s < builder.action().size(); ++s) {
        std::cout << "State " << s << ":\n";
        for (size_t a = 0; a < builder.action()[s].size(); ++a) {
            const auto& [type, target] = builder.action()[s][a];
            if (type == ActionType::kUndefined) continue;
            std::cout << "  ACTION[term " << a << "] = " << action_name(static_cast<int>(type))
                      << " " << target << "\n";
        }
        for (const auto& [sym, target] : builder.gotoT()[s]) {
            std::cout << "  GOTO[nonterm " << sym << "] = " << target << "\n";
        }
    }

    std::cout << std::flush;
}

}  // namespace
}  // namespace cc
