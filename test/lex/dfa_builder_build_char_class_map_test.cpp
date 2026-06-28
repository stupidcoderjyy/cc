//
// Created by PC on 2026/6/28.
//

#include <gtest/gtest.h>

#include "lex/dfa_builder.h"
#include "lex/nfa.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

class DfaBuilderBuildCharClassMapTest : public ::testing::Test {
protected:
    // 辅助函数：检查所有字符是否按签名正确分组
    static void CheckClasses(const std::vector<int>& char_to_class, int class_count,
                             const std::vector<CharPredicate>& predicates) {
        // 计算每个字符的签名向量，映射到类ID，验证一致性
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
    DfaBuilder builder(parser.root_node(), {});
    builder.BuildCharClassMap();

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
    DfaBuilder builder(parser.root_node(), {});
    builder.BuildCharClassMap();

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
    DfaBuilder builder(parser.root_node(), {});
    builder.BuildCharClassMap();

    // 预期仍然只有两类：digit 和 non-digit
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
    DfaBuilder builder(parser.root_node(), {});
    builder.BuildCharClassMap();

    // 四种组合：digit, alpha, space, other
    EXPECT_EQ(builder.class_count(), 4);
    auto digit = [](int c) { return c >= '0' && c <= '9'; };
    auto alpha = [](int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };
    auto space = [](int c) { return c == ' ' || c == '\t' || c == '\n'; };
    std::vector<CharPredicate> predicates = {digit, alpha, space};
    CheckClasses(builder.char_to_class(), builder.class_count(), predicates);
}

}  // namespace cc