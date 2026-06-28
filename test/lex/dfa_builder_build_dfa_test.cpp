//
// Created by PC on 2026/6/29.
//
#include <gtest/gtest.h>

#include "lex/dfa_builder.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

// 辅助：从解析器构建 DfaBuilder
static std::unique_ptr<DfaBuilder> MakeBuilderFromParser(NFARegexParser& parser) {
    auto builder = std::make_unique<DfaBuilder>(parser.root_node(), parser.node_id_to_token());
    builder->BuildCharClassMap();
    builder->ComputeClassRepresentativeChar();
    builder->ComputeNfaCharClassMask();
    return std::move(builder);
}

// 测试构建 "a" 的 DFA
TEST(DfaBuilderBuildTest, SingleCharA) {
    NFARegexParser parser;
    parser.Register("a", "token_a");

    auto builder = MakeBuilderFromParser(parser);
    auto [states, start_state] = builder->BuildDfaFromNfa();

    // 预期状态数：2（起始 + 接受）
    EXPECT_EQ(states.size(), 2);
    EXPECT_EQ(start_state, 0);

    auto* start = states[0].get();
    EXPECT_FALSE(start->is_accepted);

    // 获取字符类
    int class_a = builder->char_to_class()['a'];
    // 起始状态对 'a' 有转移到状态 1
    EXPECT_EQ(start->class_id_to_next[class_a], 1);
    // 其他类无转移（-1）
    for (int cid = 0; cid < builder->class_count(); ++cid) {
        if (cid != class_a) EXPECT_EQ(start->class_id_to_next[cid], -1);
    }

    auto* accept = states[1].get();
    EXPECT_TRUE(accept->is_accepted);
    EXPECT_EQ(accept->token, "token_a");
    // 接受状态无任何转移
    for (int cid = 0; cid < builder->class_count(); ++cid)
        EXPECT_EQ(accept->class_id_to_next[cid], -1);
}

// 测试构建 "a|b" 的 DFA
TEST(DfaBuilderBuildTest, OrAB) {
    NFARegexParser parser;
    parser.Register("a|b", "token_or");

    auto builder = MakeBuilderFromParser(parser);
    auto [states, start_state] = builder->BuildDfaFromNfa();

    // 预期状态数：3（起始 + 两个接受状态）
    EXPECT_EQ(states.size(), 3);
    EXPECT_EQ(start_state, 0);

    auto* start = states[0].get();
    int class_a = builder->char_to_class()['a'];
    int class_b = builder->char_to_class()['b'];

    // 起始状态对 'a' 和 'b' 有转移，且目标不同
    EXPECT_NE(start->class_id_to_next[class_a], -1);
    EXPECT_NE(start->class_id_to_next[class_b], -1);
    EXPECT_NE(start->class_id_to_next[class_a], start->class_id_to_next[class_b]);

    // 验证两个目标都是接受状态
    int target_a = start->class_id_to_next[class_a];
    int target_b = start->class_id_to_next[class_b];
    EXPECT_TRUE(states[target_a]->is_accepted);
    EXPECT_TRUE(states[target_b]->is_accepted);
    EXPECT_EQ(states[target_a]->token, "token_or");
    EXPECT_EQ(states[target_b]->token, "token_or");

    // 接受状态无转移
    for (int cid = 0; cid < builder->class_count(); ++cid) {
        EXPECT_EQ(states[target_a]->class_id_to_next[cid], -1);
        EXPECT_EQ(states[target_b]->class_id_to_next[cid], -1);
    }
}

// 测试构建 "a*" 的 DFA（含 ε 边）
TEST(DfaBuilderBuildTest, StarA) {
    NFARegexParser parser;
    parser.Register("a*", "token_star");

    auto builder = MakeBuilderFromParser(parser);
    auto [states, start_state] = builder->BuildDfaFromNfa();

    // 状态数至少为 2（一个起始+接受状态，一个接受状态（或循环））
    EXPECT_GE(states.size(), 2);
    EXPECT_EQ(start_state, 0);

    auto* start = states[0].get();
    // 起始状态应为接受（因为 * 允许零次）
    EXPECT_TRUE(start->is_accepted);

    int class_a = builder->char_to_class()['a'];
    int target = start->class_id_to_next[class_a];
    EXPECT_NE(target, -1);

    // 目标状态也应为接受，且对 'a' 转移回自身或另一个接受状态（循环）
    EXPECT_TRUE(states[target]->is_accepted);
    EXPECT_EQ(states[target]->class_id_to_next[class_a], target);
}

// 测试构建 "ab" 的 DFA（连接）
TEST(DfaBuilderBuildTest, ConcatAB) {
    NFARegexParser parser;
    parser.Register("ab", "token_ab");

    auto builder = MakeBuilderFromParser(parser);
    auto [states, start_state] = builder->BuildDfaFromNfa();

    // 预期状态数：3（起始 -> 中间 -> 接受）
    EXPECT_EQ(states.size(), 3);
    EXPECT_EQ(start_state, 0);

    auto* start = states[0].get();
    int class_a = builder->char_to_class()['a'];
    int class_b = builder->char_to_class()['b'];

    // 起始对 'a' 有转移，对 'b' 无
    EXPECT_NE(start->class_id_to_next[class_a], -1);
    EXPECT_EQ(start->class_id_to_next[class_b], -1);

    // 中间状态对 'b' 有转移，对 'a' 无
    int mid_id = start->class_id_to_next[class_a];
    auto* mid = states[mid_id].get();
    EXPECT_NE(mid->class_id_to_next[class_b], -1);
    EXPECT_EQ(mid->class_id_to_next[class_a], -1);

    // 接受状态无转移
    int accept_id = mid->class_id_to_next[class_b];
    auto* accept = states[accept_id].get();
    EXPECT_TRUE(accept->is_accepted);
    for (int cid = 0; cid < builder->class_count(); ++cid)
        EXPECT_EQ(accept->class_id_to_next[cid], -1);
}

}  // namespace cc