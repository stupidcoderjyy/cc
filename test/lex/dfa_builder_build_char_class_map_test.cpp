//
// Created by PC on 2026/6/28.
//

#include <gtest/gtest.h>

#include "lex/dfa_builder.h"
#include "lex/nfa.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

class DfaBuilderBuildCharClassMapTest : public testing::Test {
protected:
    // 辅助函数：检查所有字符是否按签名正确分组
    static void CheckClasses(const std::vector<int>& char_to_class,
            int class_count,
            const std::vector<CharPredicate>& predicates) {
        std::unordered_map<std::vector<bool>, int> signature_to_class;
        for (int c = 0; c < kMaxChars; ++c) {
            std::vector<bool> sig;
            for (const auto& pred : predicates) {
                sig.push_back(pred(c));
            }
            if (auto it = signature_to_class.find(sig); it == signature_to_class.end()) {
                signature_to_class[sig] = char_to_class[c];
            } else {
                EXPECT_EQ(it->second, char_to_class[c])
                        << "Character " << c << " has inconsistent class mapping.";
            }
        }
        EXPECT_EQ(signature_to_class.size(), class_count)
                << "Number of classes does not match number of distinct signatures.";
    }
};

// 用例1：单一字符谓词（数字）
TEST_F(DfaBuilderBuildCharClassMapTest, SinglePredicate) {
    NFARegexParser parser;
    parser.Register("[0-9]", "");
    DFABuilder builder(parser);
    builder.Build();

    EXPECT_EQ(builder.class_count(), 2);
    auto pred = [](int c) { return c >= '0' && c <= '9'; };
    std::vector<CharPredicate> predicates = {pred};
    CheckClasses(builder.char_to_class(), builder.class_count(), predicates);
}

// 用例2：两个不同谓词（数字和字母）
TEST_F(DfaBuilderBuildCharClassMapTest, TwoDistinctPredicates) {
    NFARegexParser parser;
    parser.Register("[0-9]", "");
    parser.Register("[a-zA-Z]", "");
    DFABuilder builder(parser);
    builder.Build();

    EXPECT_EQ(builder.class_count(), 3);  // digit, alpha, other
    auto digit = [](int c) { return c >= '0' && c <= '9'; };
    auto alpha = [](int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };
    std::vector<CharPredicate> predicates = {digit, alpha};
    CheckClasses(builder.char_to_class(), builder.class_count(), predicates);
}

// 用例3：两个相同谓词（数字两次）——应合并为一类
TEST_F(DfaBuilderBuildCharClassMapTest, SamePredicates) {
    NFARegexParser parser;
    parser.Register("[0-9]", "");
    parser.Register("[0-9]", "");
    DFABuilder builder(parser);
    builder.Build();

    EXPECT_EQ(builder.class_count(), 2);
    auto digit1 = [](int c) { return c >= '0' && c <= '9'; };
    auto digit2 = [](int c) { return c >= '0' && c <= '9'; };
    std::vector<CharPredicate> predicates = {digit1, digit2};
    CheckClasses(builder.char_to_class(), builder.class_count(), predicates);
}

// 用例4：三个不同谓词（数字、字母、空白）
TEST_F(DfaBuilderBuildCharClassMapTest, ThreeDistinctPredicates) {
    NFARegexParser parser;
    parser.Register("[0-9]", "");
    parser.Register("[a-zA-Z]", "");
    parser.Register("[ \t\n]", "");
    DFABuilder builder(parser);
    builder.Build();

    EXPECT_EQ(builder.class_count(), 4);
    auto digit = [](int c) { return c >= '0' && c <= '9'; };
    auto alpha = [](int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };
    auto space = [](int c) { return c == ' ' || c == '\t' || c == '\n'; };
    std::vector<CharPredicate> predicates = {digit, alpha, space};
    CheckClasses(builder.char_to_class(), builder.class_count(), predicates);
}

// 用例5：复杂组合（数字、标识符、字符串、单字符运算符合为一个字符类）
//
// RegisterSingles 将所有单字符拼为 [\+\-\*\=...] 一个字符类，共产生 6 个谓词：
//   p1: [0-9]          → 数字
//   p2: [a-zA-Z_]       → 标识符首字符
//   p3: [a-zA-Z0-9_]   → 标识符后续字符（是 p2 的超集）
//   p4: "              → 引号本身
//   p5: [^"]           → 非引号（p4 的取反）
//   p6: [\+\-\*\=...] → 单字符运算符合集
//
// 各字符的签名（p1-p6）：
//   数字 '0'-'9'     : {T, F, T, F, T, F}  → class 0
//   字母 'a'-'Z', '_': {F, T, T, F, T, F}  → class 1
//   引号 '"'          : {F, F, F, T, F, F}  → class 2
//   运算符 + - * ...  : {F, F, F, F, T, T}  → class 3
//   其他（空格、NUL）  : {F, F, F, F, T, F}  → class 4
TEST_F(DfaBuilderBuildCharClassMapTest, MultiplePredicates) {
    NFARegexParser parser;
    parser.Register(R"([0-9]+)", "number");
    parser.Register(R"([a-zA-Z_][a-zA-Z0-9_]*)", "alnum");
    parser.Register(R"("[^"]*")", "string");
    parser.RegisterSingles(
            {'+', '-', '*', '/', '=', '<', '>', '!', '(', ')', '{', '}', '[', ']', ';', ',', '.'});
    DFABuilder builder(parser);
    builder.Build();

    // 6 个互不等价的谓词产生 5 种字符签名
    EXPECT_EQ(builder.class_count(), 5);

    auto p_digit = [](int c) { return c >= '0' && c <= '9'; };
    auto p_alpha_us = [](int c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    };
    auto p_alnum_us = [](int c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
               c == '_';
    };
    auto p_quote = [](int c) { return c == '"'; };
    auto p_not_quote = [](int c) { return c != '"'; };
    auto p_singles = [](int c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' ||
               c == '!' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
               c == ';' || c == ',' || c == '.';
    };

    std::vector<CharPredicate> predicates = {
            p_digit, p_alpha_us, p_alnum_us, p_quote, p_not_quote, p_singles};
    CheckClasses(builder.char_to_class(), builder.class_count(), predicates);
}

}  // namespace cc
