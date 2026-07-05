//
// Created by PC on 2026/6/30.
//

#include "compile/lexer.h"

#include <gtest/gtest.h>

#include "compile/compiler_input.h"
#include "lex/dfa_builder.h"
#include "lex/dfa_setter.h"
#include "lex/nfa_regex_parser.h"

namespace cc::test {

// Token types for word-level tokens
constexpr int kTokenId = 256;
constexpr int kTokenString = 257;
constexpr int kTokenTerminal = 258;
constexpr int kTokenProdMark = 259;
constexpr int kTokenSymbMark = 260;
constexpr int kTokenTokenBegin = 261;
constexpr int kTokenSyntaxBegin = 262;
constexpr int kTokenBlockEnd = 263;

using common::CompilerInput;
using common::Lexer;
using common::LexerDataSupplier;
using common::Token;
using common::TokenEof;
using common::TokenMatchResult;
using common::TokenSingle;
using common::TokenSupplier;

// Token for 'id' rule: \a\w*
struct TokenId : Token {
    std::string val;
    int Type() override { return kTokenId; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& /*ci*/) override {
        val = lexeme;
        return TokenMatchResult::kAccept;
    }
};

// Token for 'string' rule: "[^"]*"
struct TokenStringVal : Token {
    std::string val;
    int Type() override { return kTokenString; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& /*ci*/) override {
        // Strip quotes
        val = lexeme.substr(1, lexeme.size() - 2);
        return TokenMatchResult::kAccept;
    }
};

// Token for 'terminal' rule: @($\a+|\a+|~)|'(.|\\.)'
struct TokenTerminal : Token {
    std::string val;
    int Type() override { return kTokenTerminal; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& /*ci*/) override {
        val = lexeme;
        return TokenMatchResult::kAccept;
    }
};

// Token for 'prod_mark' rule: %\d*[rRlL]?
struct TokenProdMark : Token {
    std::string val;
    int Type() override { return kTokenProdMark; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& /*ci*/) override {
        val = lexeme;
        return TokenMatchResult::kAccept;
    }
};

// Token for 'symb_mark' rule: $\d*[rRlL]?
struct TokenSymbMark : Token {
    std::string val;
    int Type() override { return kTokenSymbMark; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& /*ci*/) override {
        val = lexeme;
        return TokenMatchResult::kAccept;
    }
};

// Keyword tokens
struct TokenTokenBegin : Token {
    int Type() override { return kTokenTokenBegin; }
};

struct TokenSyntaxBegin : Token {
    int Type() override { return kTokenSyntaxBegin; }
};

struct TokenBlockEnd : Token {
    int Type() override { return kTokenBlockEnd; }
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
    void DFASetCharClassCount(int count) override { char_class_count_ = count; }
    void DFASetStatesCount(int count) override {
        states_count_ = count;
        accepted_.resize(states_count_, false);
        goto_.assign(states_count_, std::vector(char_class_count_, -1));
        token_suppliers_.resize(states_count_);
        char_to_class_.resize(kMaxChars, 0);
    }
    void DFASetStartState(int id) override { start_state_ = id; }
    void DFASetCharToClass(int ch, int class_id) override { char_to_class_[ch] = class_id; }
    void DFASetStateInfo(int state, bool accepted, const std::string& token) override {
        accepted_[state] = accepted;
        if (accepted) {
            token_suppliers_[state] = MakeSupplier(token);
        }
    }
    void DFASetGoto(int start, int input, int target) override { goto_[start][input] = target; }

    void DFAFinish() override { std::cout.flush(); }

private:
    int char_class_count_{};
    int states_count_{};
    int start_state_{};
    std::vector<bool> accepted_;
    std::vector<std::vector<int>> goto_;
    std::vector<int> char_to_class_;
    std::vector<TokenSupplier> token_suppliers_;

    static TokenSupplier MakeSupplier(const std::string& token) {
        if (token == "id") return [] { return std::make_unique<TokenId>(); };
        if (token == "string") return [] { return std::make_unique<TokenStringVal>(); };
        if (token == "terminal") return [] { return std::make_unique<TokenTerminal>(); };
        if (token == "prod_mark") return [] { return std::make_unique<TokenProdMark>(); };
        if (token == "symb_mark") return [] { return std::make_unique<TokenSymbMark>(); };
        if (token == "token_begin") return [] { return std::make_unique<TokenTokenBegin>(); };
        if (token == "syntax_begin") return [] { return std::make_unique<TokenSyntaxBegin>(); };
        if (token == "block_end") return [] { return std::make_unique<TokenBlockEnd>(); };
        if (token == "eof") return [] { return std::make_unique<TokenEof>(); };
        if (token == "single") return [] { return std::make_unique<TokenSingle>(); };
        return nullptr;
    }
};

// ==================== Test Fixture ====================

class LexerTest : public testing::Test {
protected:
    void SetUp() override {
        // Register regex patterns
        parser_.Register(R"(\a\w*)", "id");
        parser_.Register(R"("([^"\\]|\\"?)*")", "string");
        parser_.Register(R"(%%TOKEN)", "token_begin");
        parser_.Register(R"(%%SYNTAX)", "syntax_begin");
        parser_.Register(R"(%%)", "block_end");
        parser_.Register(R"(@($\a+|\a+|~)|'(.|\\.)')", "terminal");
        parser_.Register(R"(%\d*[rRlL]?)", "prod_mark");
        parser_.Register(R"($\d*[rRlL]?)", "symb_mark");
        parser_.RegisterSingles({':', ';', '|'});

        // Build DFA and populate lexer via custom setter
        TestDataSupplier tds;
        DFABuilder builder(parser_);
        builder.set_print_debug_info(true);
        // builder.set_disable_dfa_minimize_(true);
        builder.Build(&tds);
        lexer_ = std::make_unique<Lexer>(tds);
    }

    std::unique_ptr<Token> NextToken(const std::string& input) const {
        auto ci = CompilerInput::FromString(input);
        return lexer_->NextToken(*ci);
    }

    NFARegexParser parser_;
    std::unique_ptr<Lexer> lexer_;
};

TEST_F(LexerTest, IdSimple) {
    auto tok = NextToken("hello");
    ASSERT_NE(tok, nullptr);
    auto& id = tok->Cast<TokenId>();
    EXPECT_EQ(id.Type(), kTokenId);
    EXPECT_EQ(id.val, "hello");
}

TEST_F(LexerTest, IdWithUnderscore) {
    auto tok = NextToken("hello_world");
    ASSERT_NE(tok, nullptr);
    auto& id = tok->Cast<TokenId>();
    EXPECT_EQ(id.Type(), kTokenId);
    EXPECT_EQ(id.val, "hello_world");
}

TEST_F(LexerTest, IdStartingWithLetter) {
    auto tok = NextToken("a1");
    ASSERT_NE(tok, nullptr);
    auto& id = tok->Cast<TokenId>();
    EXPECT_EQ(id.Type(), kTokenId);
    EXPECT_EQ(id.val, "a1");
}

TEST_F(LexerTest, IdWithDigits) {
    auto tok = NextToken("var123");
    ASSERT_NE(tok, nullptr);
    auto& id = tok->Cast<TokenId>();
    EXPECT_EQ(id.Type(), kTokenId);
    EXPECT_EQ(id.val, "var123");
}

// ==================== String Tests ====================

TEST_F(LexerTest, StringEmpty) {
    auto tok = NextToken(R"("")");
    ASSERT_NE(tok, nullptr);
    auto& str = tok->Cast<TokenStringVal>();
    EXPECT_EQ(str.Type(), kTokenString);
    EXPECT_EQ(str.val, "");
}

TEST_F(LexerTest, StringWithContent) {
    auto tok = NextToken(R"("abc def")");
    ASSERT_NE(tok, nullptr);
    auto& str = tok->Cast<TokenStringVal>();
    EXPECT_EQ(str.Type(), kTokenString);
    EXPECT_EQ(str.val, "abc def");
}

TEST_F(LexerTest, StringWithEscapedQuote) {
    auto tok = NextToken(R"("a\"b\")");
    ASSERT_NE(tok, nullptr);
    auto& str = tok->Cast<TokenStringVal>();
    EXPECT_EQ(str.Type(), kTokenString);
    EXPECT_EQ(str.val, R"(a\"b\)");
}

// ==================== Keyword Tests ====================

TEST_F(LexerTest, TokenBegin) {
    auto tok = NextToken("%%TOKEN");
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(tok->Type(), kTokenTokenBegin);
}

TEST_F(LexerTest, SyntaxBegin) {
    auto tok = NextToken("%%SYNTAX");
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(tok->Type(), kTokenSyntaxBegin);
}

TEST_F(LexerTest, BlockEnd) {
    auto tok = NextToken("%%");
    ASSERT_NE(tok, nullptr);
    EXPECT_EQ(tok->Type(), kTokenBlockEnd);
}

// ==================== Terminal Tests ====================

TEST_F(LexerTest, TerminalAtSimple) {
    auto tok = NextToken("@a");
    ASSERT_NE(tok, nullptr);
    auto& term = tok->Cast<TokenTerminal>();
    EXPECT_EQ(term.Type(), kTokenTerminal);
    EXPECT_EQ(term.val, "@a");
}

TEST_F(LexerTest, TerminalAtDollar) {
    auto tok = NextToken("@$abc");
    ASSERT_NE(tok, nullptr);
    auto& term = tok->Cast<TokenTerminal>();
    EXPECT_EQ(term.Type(), kTokenTerminal);
    EXPECT_EQ(term.val, "@$abc");
}

TEST_F(LexerTest, TerminalAtTilde) {
    auto tok = NextToken("@~");
    ASSERT_NE(tok, nullptr);
    auto& term = tok->Cast<TokenTerminal>();
    EXPECT_EQ(term.Type(), kTokenTerminal);
    EXPECT_EQ(term.val, "@~");
}

TEST_F(LexerTest, TerminalSingleChar) {
    auto tok = NextToken("'a'");
    ASSERT_NE(tok, nullptr);
    auto& term = tok->Cast<TokenTerminal>();
    EXPECT_EQ(term.Type(), kTokenTerminal);
    EXPECT_EQ(term.val, "'a'");
}

TEST_F(LexerTest, TerminalSingleEscaped) {
    auto tok = NextToken("'\\n'");
    ASSERT_NE(tok, nullptr);
    auto& term = tok->Cast<TokenTerminal>();
    EXPECT_EQ(term.Type(), kTokenTerminal);
    EXPECT_EQ(term.val, "'\\n'");
}

// ==================== Prod Mark Tests ====================

TEST_F(LexerTest, ProdMarkSimple) {
    auto tok = NextToken("%12L");
    ASSERT_NE(tok, nullptr);
    auto& pm = tok->Cast<TokenProdMark>();
    EXPECT_EQ(pm.Type(), kTokenProdMark);
    EXPECT_EQ(pm.val, "%12L");
}

TEST_F(LexerTest, ProdMarkNoDigits) {
    auto tok = NextToken("%R");
    ASSERT_NE(tok, nullptr);
    auto& pm = tok->Cast<TokenProdMark>();
    EXPECT_EQ(pm.Type(), kTokenProdMark);
    EXPECT_EQ(pm.val, "%R");
}

TEST_F(LexerTest, ProdMarkOnlyPercent) {
    // According to rule %\d*[rRlL]?, the '?' makes the letter optional, so "%" is valid.
    auto tok = NextToken("%");
    ASSERT_NE(tok, nullptr);
    auto& pm = tok->Cast<TokenProdMark>();
    EXPECT_EQ(pm.Type(), kTokenProdMark);
    EXPECT_EQ(pm.val, "%");
}

// ==================== Symb Mark Tests ====================

TEST_F(LexerTest, SymbMarkSimple) {
    auto tok = NextToken("$12L");
    ASSERT_NE(tok, nullptr);
    auto& sm = tok->Cast<TokenSymbMark>();
    EXPECT_EQ(sm.Type(), kTokenSymbMark);
    EXPECT_EQ(sm.val, "$12L");
}

TEST_F(LexerTest, SymbMarkNoDigits) {
    auto tok = NextToken("$R");
    ASSERT_NE(tok, nullptr);
    auto& sm = tok->Cast<TokenSymbMark>();
    EXPECT_EQ(sm.Type(), kTokenSymbMark);
    EXPECT_EQ(sm.val, "$R");
}

TEST_F(LexerTest, SymbMarkOnlyDollar) {
    auto tok = NextToken("$");
    ASSERT_NE(tok, nullptr);
    auto& sm = tok->Cast<TokenSymbMark>();
    EXPECT_EQ(sm.Type(), kTokenSymbMark);
    EXPECT_EQ(sm.val, "$");
}

// ==================== Edge Cases ====================
TEST_F(LexerTest, UnknownCharProducesNull) {
    auto tok = NextToken("^");
    EXPECT_EQ(tok, nullptr);
}

TEST_F(LexerTest, UnclosedString) {
    // According to the regex, an unclosed string will not match the string rule,
    // but may be recognized as something else? Actually it will fail.
    auto tok = NextToken("\"abc");
    EXPECT_EQ(tok, nullptr);
}

TEST_F(LexerTest, LongInput) {
    std::string input = "%%TOKEN %%SYNTAX %% hello \"world\" @a %123L $456R";
    // Expect sequence of tokens; just test the first few.
    auto ci = CompilerInput::FromString(input);
    auto tok1 = lexer_->NextToken(*ci);
    ASSERT_NE(tok1, nullptr);
    EXPECT_EQ(tok1->Type(), kTokenTokenBegin);

    auto tok2 = lexer_->NextToken(*ci);
    ASSERT_NE(tok2, nullptr);
    EXPECT_EQ(tok2->Type(), kTokenSyntaxBegin);

    auto tok3 = lexer_->NextToken(*ci);
    ASSERT_NE(tok3, nullptr);
    EXPECT_EQ(tok3->Type(), kTokenBlockEnd);

    auto tok4 = lexer_->NextToken(*ci);
    ASSERT_NE(tok4, nullptr);
    auto& id = tok4->Cast<TokenId>();
    EXPECT_EQ(id.Type(), kTokenId);
    EXPECT_EQ(id.val, "hello");

    auto tok5 = lexer_->NextToken(*ci);
    ASSERT_NE(tok5, nullptr);
    auto& str = tok5->Cast<TokenStringVal>();
    EXPECT_EQ(str.Type(), kTokenString);
    EXPECT_EQ(str.val, "world");
}
// ==================== Singles ====================

TEST_F(LexerTest, Singles) {
    std::string input = " : ; | ";
    auto ci = CompilerInput::FromString(input);
    auto tok1 = lexer_->NextToken(*ci);
    ASSERT_NE(tok1, nullptr);
    EXPECT_EQ(tok1->Type(), ':');

    auto tok2 = lexer_->NextToken(*ci);
    ASSERT_NE(tok2, nullptr);
    EXPECT_EQ(tok2->Type(), ';');

    auto tok3 = lexer_->NextToken(*ci);
    ASSERT_NE(tok3, nullptr);
    EXPECT_EQ(tok3->Type(), '|');
}
}  // namespace cc::test
