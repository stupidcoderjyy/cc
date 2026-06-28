//
// Created by PC on 2026/6/26.
//

#include <algorithm>
#include <gtest/gtest.h>
#include <unordered_set>
#include <queue>

#include "lex/nfa_regex_parser.h"
#include "lex/nfa_node.h"

namespace cc {

// ---------- NFA 模拟器（用于测试）----------
class NFASimulator {
public:
    explicit NFASimulator(NFANode* start) : start_(start) {}

    bool Match(const std::string& input) {
        // 初始状态集合：从 start 出发的 ε-闭包
        auto current = EpsilonClosure({start_});

        for (char ch : input) {
            auto next = Move(current, ch);
            if (next.empty()) return false;
            current = EpsilonClosure(next);
        }

        // 检查是否有接受状态
        return std::ranges::any_of(current, [](auto* state) {
            return state->accepted();
        });
    }

private:
    static std::unordered_set<NFANode*> EpsilonClosure(const std::unordered_set<NFANode*>& states) {
        std::unordered_set<NFANode*> closure = states;
        std::queue<NFANode*> q;
        for (auto s : states) q.push(s);

        while (!q.empty()) {
            auto* s = q.front();
            q.pop();
            // 检查 epsilon 边
            if (s->edge_type() == NFANode::EdgeType::kSingleEpsilon) {
                if (auto* next = s->next1(); next && !closure.contains(next)) {
                    closure.insert(next);
                    q.push(next);
                }
            } else if (s->edge_type() == NFANode::EdgeType::kDoubleEpsilon) {
                NFANode* n1 = s->next1();
                NFANode* n2 = s->next2();
                if (n1 && !closure.contains(n1)) {
                    closure.insert(n1);
                    q.push(n1);
                }
                if (n2 && !closure.contains(n2)) {
                    closure.insert(n2);
                    q.push(n2);
                }
            }
        }
        return closure;
    }

    static std::unordered_set<NFANode*> Move(const std::unordered_set<NFANode*>& states, int ch) {
        std::unordered_set<NFANode*> next;
        for (NFANode* s : states) {
            if (s->edge_type() == NFANode::EdgeType::kChar) {
                if (s->predicate() && s->predicate()(ch)) {
                    next.insert(s->next1());
                }
            }
        }
        return next;
    }

    NFANode* start_;
};

// ---------- 测试夹具 ----------
class NFARegexParserTest : public ::testing::Test {
protected:
    static bool MatchRegex(const std::string& regex, const std::string& input) {
        NFARegexParser local_parser;
        local_parser.Register(regex, "test");
        NFASimulator sim(local_parser.root_node());
        return sim.Match(input);
    }
};

// ---------- 基本字符匹配 ----------
TEST_F(NFARegexParserTest, SingleChar) {
    EXPECT_TRUE(MatchRegex(R"(a)", "a"));
    EXPECT_FALSE(MatchRegex(R"(a)", "b"));
    EXPECT_FALSE(MatchRegex(R"(a)", ""));
}

TEST_F(NFARegexParserTest, Concatenation) {
    EXPECT_TRUE(MatchRegex(R"(abc)", "abc"));
    EXPECT_FALSE(MatchRegex(R"(abc)", "ab"));
    EXPECT_FALSE(MatchRegex(R"(abc)", "abcd"));
    EXPECT_FALSE(MatchRegex(R"(abc)", "a c"));
}

// ---------- 选择（|） ----------
TEST_F(NFARegexParserTest, Or) {
    EXPECT_TRUE(MatchRegex(R"(a|b)", "a"));
    EXPECT_TRUE(MatchRegex(R"(a|b)", "b"));
    EXPECT_FALSE(MatchRegex(R"(a|b)", "c"));
    EXPECT_TRUE(MatchRegex(R"(ab|cd)", "ab"));
    EXPECT_TRUE(MatchRegex(R"(ab|cd)", "cd"));
    EXPECT_FALSE(MatchRegex(R"(ab|cd)", "ac"));
}

// ---------- 重复运算符 ----------
TEST_F(NFARegexParserTest, Star) {
    EXPECT_TRUE(MatchRegex(R"(a*)", ""));
    EXPECT_TRUE(MatchRegex(R"(a*)", "a"));
    EXPECT_TRUE(MatchRegex(R"(a*)", "aaa"));
    EXPECT_FALSE(MatchRegex(R"(a*)", "b"));
    EXPECT_TRUE(MatchRegex(R"(ab*)", "a"));
    EXPECT_TRUE(MatchRegex(R"(ab*)", "ab"));
    EXPECT_TRUE(MatchRegex(R"(ab*)", "abbb"));
    EXPECT_FALSE(MatchRegex(R"(ab*)", "abbc"));
}

TEST_F(NFARegexParserTest, Plus) {
    EXPECT_FALSE(MatchRegex(R"(a+)", ""));
    EXPECT_TRUE(MatchRegex(R"(a+)", "a"));
    EXPECT_TRUE(MatchRegex(R"(a+)", "aaa"));
    EXPECT_FALSE(MatchRegex(R"(a+)", "b"));
}

TEST_F(NFARegexParserTest, Quest) {
    EXPECT_TRUE(MatchRegex(R"(a?)", ""));
    EXPECT_TRUE(MatchRegex(R"(a?)", "a"));
    EXPECT_FALSE(MatchRegex(R"(a?)", "aa"));
    EXPECT_TRUE(MatchRegex(R"(ab?)", "a"));
    EXPECT_TRUE(MatchRegex(R"(ab?)", "ab"));
    EXPECT_FALSE(MatchRegex(R"(ab?)", "abb"));
}

// ---------- 字符类 ----------
TEST_F(NFARegexParserTest, CharClass) {
    EXPECT_TRUE(MatchRegex(R"([abc])", "a"));
    EXPECT_TRUE(MatchRegex(R"([abc])", "b"));
    EXPECT_TRUE(MatchRegex(R"([abc])", "c"));
    EXPECT_FALSE(MatchRegex(R"([abc])", "d"));
    EXPECT_FALSE(MatchRegex(R"([abc])", "ab"));

    EXPECT_TRUE(MatchRegex(R"([a-z])", "m"));
    EXPECT_FALSE(MatchRegex(R"([a-z])", "1"));

    EXPECT_TRUE(MatchRegex(R"([^0-9])", "a"));
    EXPECT_FALSE(MatchRegex(R"([^0-9])", "5"));

    EXPECT_TRUE(MatchRegex(R"([\n])", "\n"));
    EXPECT_TRUE(MatchRegex(R"([\t])", "\t"));
    EXPECT_TRUE(MatchRegex(R"([\x])", "x"));

    EXPECT_THROW(MatchRegex(R"([^])", "a"), std::exception);
}

TEST_F(NFARegexParserTest, CharClassNested) {
    EXPECT_TRUE(MatchRegex(R"([a-d[0-3]])", "b"));
    EXPECT_TRUE(MatchRegex(R"([a-d[0-3]])", "2"));
    EXPECT_FALSE(MatchRegex(R"([a-d[0-3]])", "z"));
}

// ---------- 转义字符 ----------
TEST_F(NFARegexParserTest, Escape) {
    EXPECT_TRUE(MatchRegex(R"(\d)", "5"));
    EXPECT_FALSE(MatchRegex(R"(\d)", "a"));
    EXPECT_TRUE(MatchRegex(R"(\w)", "A"));
    EXPECT_TRUE(MatchRegex(R"(\w)", "_"));
    EXPECT_FALSE(MatchRegex(R"(\w)", "!"));
    EXPECT_TRUE(MatchRegex(R"(\n)", "\n"));
    EXPECT_TRUE(MatchRegex(R"(\t)", "\t"));
    EXPECT_TRUE(MatchRegex(R"([\x])", "x"));
}

// ---------- 分组 ----------
TEST_F(NFARegexParserTest, Group) {
    EXPECT_TRUE(MatchRegex(R"((ab))", "ab"));
    EXPECT_FALSE(MatchRegex(R"((ab))", "a"));
    EXPECT_TRUE(MatchRegex(R"((ab)*)", "abab"));
    EXPECT_TRUE(MatchRegex(R"(a(bc)*d)", "abcd"));
    EXPECT_TRUE(MatchRegex(R"(a(bc)*d)", "abcbcbcbcd"));
    EXPECT_FALSE(MatchRegex(R"(a(bc)*d)", "abbd"));
}

// ---------- 复杂组合 ----------
TEST_F(NFARegexParserTest, Complex) {
    EXPECT_TRUE(MatchRegex(R"(a(b|c)d)", "abd"));
    EXPECT_TRUE(MatchRegex(R"(a(b|c)d)", "acd"));
    EXPECT_FALSE(MatchRegex(R"(a(b|c)d)", "abcd"));
    EXPECT_TRUE(MatchRegex(R"((0|1|2|3|4|5|6|7|8|9)*)", "12345"));
    EXPECT_TRUE(MatchRegex(R"((0|1|2|3|4|5|6|7|8|9)*)", ""));
    EXPECT_FALSE(MatchRegex(R"((0|1|2|3|4|5|6|7|8|9)*)", "12a"));
}

// ---------- 多个 Register 并联 ----------
TEST_F(NFARegexParserTest, MultipleRegister) {
    NFARegexParser parser;
    parser.Register(R"(abc)", "token1");
    parser.Register(R"(def)", "token2");
    NFASimulator sim(parser.root_node());
    EXPECT_TRUE(sim.Match("abc"));
    EXPECT_TRUE(sim.Match("def"));
}

// ---------- 错误输入（异常） ----------
TEST_F(NFARegexParserTest, InvalidRegex) {
    NFARegexParser parser;
    EXPECT_THROW(parser.Register(R"((a)", "test"), std::exception);
    EXPECT_THROW(parser.Register(R"(a))", "test"), std::exception);
    EXPECT_THROW(parser.Register(R"(a|)", "test"), std::exception);
    EXPECT_THROW(parser.Register(R"(a*+)", "test"), std::exception);
}

}