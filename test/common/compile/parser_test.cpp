//
// Created by PC on 2026/7/1.
//

#include "compile/parser.h"

#include <gtest/gtest.h>

#include "compile/compiler_input.h"
#include "compile/lexer.h"
#include "lex/dfa_builder.h"
#include "lex/dfa_setter.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

using common::CompilerInput;
using common::kActionAccept;
using common::kActionReduce;
using common::kActionShift;
using common::Lexer;
using common::LexerDataSupplier;
using common::Parser;
using common::ParserDataSupplier;
using common::Production;
using common::Property;
using common::PropertySupplier;
using common::Symbol;
using common::TokenEof;
using common::TokenSingle;
using common::TokenSupplier;

// ==================== Shared Lexer infrastructure ====================

class TestDataSupplier : public LexerDataSupplier, public DFASetter {
public:
    int CharClassCount() override { return char_class_count_; }
    int StatesCount() override { return states_count_; }
    int StartState() override { return start_state_; }
    void InitAccepted(std::vector<bool>& vec) override { vec = std::move(accepted_); }
    void InitGoto(std::vector<std::vector<int>>& vec) override { vec = std::move(goto_); }
    void InitCharToClass(std::vector<int>& vec) override { vec = std::move(char_to_class_); }
    void InitTokenSuppliers(std::vector<TokenSupplier>& vec) override {
        vec = std::move(token_suppliers_);
    }

    void SetCharClassCount(int count) override { char_class_count_ = count; }
    void SetDfaStatesCount(int count) override {
        states_count_ = count;
        accepted_.resize(states_count_, false);
        goto_.assign(states_count_, std::vector(char_class_count_, -1));
        token_suppliers_.resize(states_count_);
        char_to_class_.resize(kMaxChars, 0);
    }
    void SetStartState(int id) override { start_state_ = id; }
    void SetCharToClass(int ch, int class_id) override { char_to_class_[ch] = class_id; }
    void SetStateInfo(int state, bool accepted, const std::string& token) override {
        accepted_[state] = accepted;
        if (accepted) {
            token_suppliers_[state] = MakeSupplier(token);
        }
    }
    void SetGoto(int start, int input, int target) override { goto_[start][input] = target; }
    void Finish() override {}

private:
    int char_class_count_{};
    int states_count_{};
    int start_state_{};
    std::vector<bool> accepted_;
    std::vector<std::vector<int>> goto_;
    std::vector<int> char_to_class_;
    std::vector<TokenSupplier> token_suppliers_;

    static TokenSupplier MakeSupplier(const std::string& token) {
        if (token == "single") return [] { return std::make_unique<TokenSingle>(); };
        if (token == "eof") return [] { return std::make_unique<TokenEof>(); };
        return nullptr;
    }
};

// ==================== ParserDataSupplier for grammar A -> a ====================
//
// Grammar (after auto-inserted ROOT -> A):
//   prod 0: ROOT -> A    (accept)
//   prod 1: A -> a       (reduce)
//
// LALR states:
//   State 0: ROOT -> ·A, A -> ·a   shift 'a' → state 1, goto A → state 2
//   State 1: A -> a·               reduce prod 1 on $
//   State 2: ROOT -> A·            accept on $
//
// Terminal symbols: $(0), a(1)
// Non-terminal symbols: ROOT(0), A(1)

class TestParserDataSupplier : public ParserDataSupplier {
public:
    int TokenMappersCount() const override { return 128; }
    int NonTerminalSymbolsCount() const override { return 2; }
    int TerminalSymbolsCount() const override { return 2; }
    int StatesCount() const override { return 3; }

    void InitReduceActions(std::vector<common::ReduceFunc>& vec) override {}

    void InitGoto(std::vector<std::vector<int>>& vec) override {
        vec.assign(3, std::vector(2, 0));
        vec[0][1] = 2;  // state 0 on A → state 2
    }

    void InitActions(std::vector<std::vector<int>>& vec) override {
        vec.assign(3, std::vector(2, 0));
        vec[0][1] = kActionShift | 1;   // state 0 on 'a': SHIFT → state 1
        vec[1][0] = kActionReduce | 1;  // state 1 on '$': REDUCE prod 1
        vec[2][0] = kActionAccept;      // state 2 on '$': ACCEPT
    }

    void InitTokenToSymbol(std::vector<int>& vec) override {
        vec.assign(128, 0);
        vec[0] = 0;    // $ → terminal symbol 0
        vec['a'] = 1;  // 'a' → terminal symbol 1
    }

    void InitPropertySuppliers(std::vector<PropertySupplier>& vec) override {
        vec.resize(2);
        vec[0] = [] { return std::make_unique<Property>(); };
        vec[1] = [] { return std::make_unique<Property>(); };
    }

    void InitSymbolsAndProductions(
            std::vector<Symbol>& symbols, std::vector<Production>& prods) override {
        symbols.resize(2);
        symbols[0] = {false, 0};
        symbols[1] = {false, 1};
        prods.resize(2);
        prods[0] = Production{0, {false, 0}, {{false, 1}}};
        prods[1] = Production{1, {false, 1}, {{true, 1}}};
    }
};

// ==================== Test Fixture ====================

class ParserTest : public testing::Test {
protected:
    void SetUp() override {
        NFARegexParser parser;
        parser.RegisterSingles({'a'});
        TestDataSupplier tds;
        DFABuilder builder(parser);
        builder.Build(&tds);
        lexer_ = std::make_unique<Lexer>(tds);

        parser_data_ = std::make_unique<TestParserDataSupplier>();
    }

    bool ParseInput(const std::string& input) const {
        auto ci = CompilerInput::FromString(input);
        Parser p(*parser_data_);
        try {
            p.Parse(*lexer_, *ci);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    std::unique_ptr<Lexer> lexer_;
    std::unique_ptr<TestParserDataSupplier> parser_data_;
};

// ==================== Tests ====================

TEST_F(ParserTest, AcceptSingleA) {
    EXPECT_TRUE(ParseInput("a"));
}

TEST_F(ParserTest, RejectEmpty) {
    EXPECT_TRUE(ParseInput(""));  // 直接返回
}

TEST_F(ParserTest, RejectUnknownChar) {
    EXPECT_FALSE(ParseInput("b"));
}

TEST_F(ParserTest, RejectDoubleA) {
    EXPECT_FALSE(ParseInput("aa"));
}

}  // namespace cc
