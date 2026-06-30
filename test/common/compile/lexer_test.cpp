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

// Subclass of Lexer that exposes protected members for population by DFASetter
class BuildableLexer : public Lexer {
public:
    using Lexer::Lexer;

    void SetAccepted(int state, bool v) {
        accepted_[state] = v;
        std::cout << std::format("accepted_[{}] = {} \n", state, v);
    }
    void SetGoto(int state, int ch, int target) {
        goto_[state][ch] = target;
        std::cout << std::format("goto_[{}][{}] = {}; \n", state, ch, target);
    }
    void SetTokenSupplier(int state, TokenSupplier supplier) {
        token_suppliers_[state] = std::move(supplier);
    }

    void SetCharToClass(int ch, int class_id) {
        char_to_class_[ch] = class_id;
        std::cout << std::format("char_to_class_['{}'] = {}; \n", static_cast<char>(ch), class_id);
    }
};

// DFASetter implementation that populates a BuildableLexer
class LexerDFASetter : public DFASetter {
public:
    LexerDFASetter() = default;

    void SetCharClassCount(int count) override { char_class_count_ = count; }

    void SetCharToClass(int ch, int class_id) override { char_to_class_[ch] = class_id; }

    void SetDfaStatesCount(int count) override { states_count_ = count; }

    void SetStartState(int id) override { start_state_ = id; }

    void SetStateInfo(int stateId, bool isAccepted, const std::string& token) override {
        accepted_[stateId] = isAccepted;
        if (isAccepted) {
            tokens_[stateId] = MakeSupplier(token);
        }
    }

    void SetTransition(int start, int input, int target) override {
        transitions_.push_back({start, input, target});
    }

    void Finish() override {
        std::cout << std::format("char_class_count_ = {}; \n", char_class_count_);
        std::cout << std::format("states_count_ = {}; \n", states_count_);
        std::cout << std::format("start_state_ = {}; \n", start_state_);
        lexer_ = std::make_unique<BuildableLexer>(char_class_count_, states_count_, start_state_);
        for (const auto& [s, b] : accepted_) {
            lexer_->SetAccepted(s, b);
        }
        for (const auto& [s, t] : tokens_) {
            lexer_->SetTokenSupplier(s, t);
        }
        for (const auto& [s, i, t] : transitions_) {
            lexer_->SetGoto(s, i, t);
        }
        for (const auto& [c, i] : char_to_class_) {
            lexer_->SetCharToClass(c, i);
        }
    }

    std::unique_ptr<BuildableLexer> lexer() { return std::move(lexer_); }

private:
    std::unique_ptr<BuildableLexer> lexer_;
    std::unordered_map<int, int> char_to_class_;
    int char_class_count_{};
    int states_count_{};
    int start_state_{};
    struct Transition {
        int start;
        int input;
        int target;
    };
    std::vector<Transition> transitions_;
    std::unordered_map<int, bool> accepted_;
    std::unordered_map<int, Lexer::TokenSupplier> tokens_;

    static Lexer::TokenSupplier MakeSupplier(const std::string& token) {
        if (token == "number") return [] { return std::make_unique<TokenNumber>(); };
        if (token == "alnum") return [] { return std::make_unique<TokenAlnum>(); };
        if (token == "string") return [] { return std::make_unique<TokenString>(); };
        if (token == "single") return [] { return std::make_unique<TokenSingle>(); };
        if (token == "eof") return [] { return std::make_unique<TokenEof>(); };
        return nullptr;
    }
};

// ==================== Test Fixture ====================

class LexerTest : public ::testing::Test {
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
        LexerDFASetter setter;
        DFABuilder builder(parser_, &setter);
        lexer_ = setter.lexer();
        std::cout.flush();
    }

    std::unique_ptr<Token> NextToken(const std::string& input) const {
        auto ci = CompilerInput::FromString(input);
        return lexer_->NextToken(*ci);
    }

    NFARegexParser parser_;
    std::unique_ptr<BuildableLexer> lexer_;
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
