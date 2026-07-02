//
// Created by PC on 2026/6/30.
//

#include "compile/lexer.h"

#include <gtest/gtest.h>

#include "compile/compiler_input.h"
#include "lex/dfa_builder.h"
#include "lex/dfa_setter.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

// Token types for word-level tokens
constexpr int kTokenNumber = 256;
constexpr int kTokenAlpha = 257;
constexpr int kTokenAlnum = 258;
constexpr int kTokenString = 259;

using common::CompilerInput;
using common::Lexer;
using common::LexerDataSupplier;
using common::Token;
using common::TokenEof;
using common::TokenMatchResult;
using common::TokenSingle;
using common::TokenSupplier;

// Custom token: number literal
struct TokenNumber : Token {
    int val{};
    int Type() override { return kTokenNumber; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override {
        val = std::stoi(lexeme);
        return TokenMatchResult::kAccept;
    }
};

// Custom token: alphanumeric identifier (with underscore)
struct TokenAlnum : Token {
    std::string val;
    int Type() override { return kTokenAlnum; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override {
        val = lexeme;
        return TokenMatchResult::kAccept;
    }
};

// Custom token: double-quoted string
struct TokenString : Token {
    std::string val;
    int Type() override { return kTokenString; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& /*ci*/) override {
        // Strip surrounding double quotes
        val = lexeme.substr(1, lexeme.size() - 2);
        return TokenMatchResult::kAccept;
    }
};

class TestDataSupplier : public LexerDataSupplier, public DFASetter {
public:
    //LexerDataSupplier
    int CharClassCount() override { return char_class_count_; }
    int StatesCount() override { return states_count_; }
    int StartState() override { return start_state_; }
    void InitAccepted(std::vector<bool>& vec) override { vec = std::move(accepted_); }
    void InitGoto(std::vector<std::vector<int>>& vec) override { vec = std::move(goto_); }
    void InitCharToClass(std::vector<int>& vec) override { vec = std::move(char_to_class_); }
    void InitTokenSuppliers(std::vector<TokenSupplier>& vec) override {
        vec = std::move(token_suppliers_);
    }

    //DFASetter
    void SetCharClassCount(int count) override { char_class_count_ = count; }
    void SetDfaStatesCount(int count) override {
        states_count_ = count;
        accepted_.resize(states_count_, false);
        goto_.assign(states_count_, std::vector(char_class_count_, -1));
        token_suppliers_.resize(states_count_);
        char_to_class_.resize(kMaxChars, 0);
    }
    void SetStartState(int id) override { start_state_ = id; }
    void SetCharToClass(int ch, int class_id) override {
        char_to_class_[ch] = class_id;
        std::cout << std::format("char_to_class_['{}'] = {}; \n", static_cast<char>(ch), class_id);
    }
    void SetStateInfo(int state, bool accepted, const std::string& token) override {
        accepted_[state] = accepted;
        if (accepted) {
            token_suppliers_[state] = MakeSupplier(token);
        }
        std::cout << std::format("accepted_[{}] = {} \n", state, accepted);
    }
    void SetGoto(int start, int input, int target) override {
        goto_[start][input] = target;
        std::cout << std::format("goto_[{}][{}] = {}; \n", start, input, target);
    }

    void Finish() override {
        std::cout << std::format("char_class_count_ = {}; \n", char_class_count_);
        std::cout << std::format("states_count_ = {}; \n", states_count_);
        std::cout << std::format("start_state_ = {}; \n", start_state_);
        std::cout.flush();
    }

private:
    int char_class_count_{};
    int states_count_{};
    int start_state_{};
    std::vector<bool> accepted_;
    std::vector<std::vector<int>> goto_;
    std::vector<int> char_to_class_;
    std::vector<TokenSupplier> token_suppliers_;

    static TokenSupplier MakeSupplier(const std::string& token) {
        if (token == "number") return [] { return std::make_unique<TokenNumber>(); };
        if (token == "alnum") return [] { return std::make_unique<TokenAlnum>(); };
        if (token == "string") return [] { return std::make_unique<TokenString>(); };
        if (token == "single") return [] { return std::make_unique<TokenSingle>(); };
        if (token == "eof") return [] { return std::make_unique<TokenEof>(); };
        return nullptr;
    }
};

// ==================== Test Fixture ====================

class LexerTest : public testing::Test {
protected:
    void SetUp() override {
        // Register regex patterns
        parser_.Register(R"([0-9]+)", "number");
        parser_.Register(R"([a-zA-Z_][a-zA-Z0-9_]*)", "alnum");
        parser_.Register(R"("[^"]*")", "string");

        // Register single-character operators
        parser_.RegisterSingles(
            {'+', '-', '*', '/', '=', '<', '>', '!', '(', ')', '{', '}', '[', ']', ';', ',', '.'});

        // Build DFA and populate lexer via custom setter
        TestDataSupplier tds;
        DFABuilder builder(parser_, &tds);
        lexer_ = std::make_unique<Lexer>(tds);
    }

    std::unique_ptr<Token> NextToken(const std::string& input) const {
        auto ci = CompilerInput::FromString(input);
        return lexer_->NextToken(*ci);
    }

    NFARegexParser parser_;
    std::unique_ptr<Lexer> lexer_;
};

// ==================== Number Tests ====================

TEST_F(LexerTest, NumberSingleDigit) {
    auto tok = NextToken("2");
    auto& num_token = tok->Cast<TokenNumber>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), kTokenNumber);
    EXPECT_EQ(num_token.val, 2);
}

TEST_F(LexerTest, NumberMultiDigit) {
    auto tok = NextToken("12345");
    auto& num_token = tok->Cast<TokenNumber>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), kTokenNumber);
    EXPECT_EQ(num_token.val, 12345);
}

// ==================== Alnum Tests ====================

TEST_F(LexerTest, AlnumSimple) {
    auto tok = NextToken("hello");
    auto& num_token = tok->Cast<TokenAlnum>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), kTokenAlnum);
    EXPECT_EQ(num_token.val, "hello");
}

TEST_F(LexerTest, AlnumWithUnderscore) {
    auto tok = NextToken("hello_world");
    auto& num_token = tok->Cast<TokenAlnum>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), kTokenAlnum);
    EXPECT_EQ(num_token.val, "hello_world");
}

TEST_F(LexerTest, AlnumWithDigits) {
    auto tok = NextToken("var123");
    auto& num_token = tok->Cast<TokenAlnum>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), kTokenAlnum);
    EXPECT_EQ(num_token.val, "var123");
}

// ==================== String Tests ====================

TEST_F(LexerTest, StringEmpty) {
    auto tok = NextToken(R"("")");
    auto& num_token = tok->Cast<TokenString>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), kTokenString);
    EXPECT_EQ(num_token.val, "");
}

TEST_F(LexerTest, StringWithContent) {
    auto tok = NextToken(R"("abc def")");
    auto& num_token = tok->Cast<TokenString>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), kTokenString);
    EXPECT_EQ(num_token.val, "abc def");
}

// ==================== Single Character Tests ====================

TEST_F(LexerTest, SinglePlus) {
    char ch = '+';
    auto tok = NextToken(std::string(1, ch));
    auto& num_token = tok->Cast<TokenSingle>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), ch);
    EXPECT_EQ(num_token.ch, ch);
}

TEST_F(LexerTest, SingleMinus) {
    char ch = '-';
    auto tok = NextToken(std::string(1, ch));
    auto& num_token = tok->Cast<TokenSingle>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), ch);
    EXPECT_EQ(num_token.ch, ch);
}

TEST_F(LexerTest, SingleStar) {
    char ch = '*';
    auto tok = NextToken(std::string(1, ch));
    auto& num_token = tok->Cast<TokenSingle>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), ch);
    EXPECT_EQ(num_token.ch, ch);
}

TEST_F(LexerTest, SingleSlash) {
    char ch = '/';
    auto tok = NextToken(std::string(1, ch));
    auto& num_token = tok->Cast<TokenSingle>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), ch);
    EXPECT_EQ(num_token.ch, ch);
}

TEST_F(LexerTest, SingleEquals) {
    char ch = '=';
    auto tok = NextToken(std::string(1, ch));
    auto& num_token = tok->Cast<TokenSingle>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token.Type(), ch);
    EXPECT_EQ(num_token.ch, ch);
}

TEST_F(LexerTest, SingleParentheses) {
    char ch = '(';
    auto tok = NextToken(std::string(1, ch));
    auto* num_token = &tok->Cast<TokenSingle>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token->Type(), ch);
    EXPECT_EQ(num_token->ch, ch);

    ch = ')';
    tok = NextToken(std::string(1, ch));
    num_token = &tok->Cast<TokenSingle>();
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(num_token->Type(), ch);
    EXPECT_EQ(num_token->ch, ch);
}

// ==================== Edge Case Tests ====================
TEST_F(LexerTest, UnknownCharProducesNull) {
    auto tok = NextToken("^");
    EXPECT_EQ(tok, nullptr);
}

}  // namespace cc
