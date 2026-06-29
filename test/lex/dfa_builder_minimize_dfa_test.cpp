//
// Created by PC on 2026/6/29.
//
#include <gtest/gtest.h>

#include "lex/dfa_builder.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

// 辅助函数：从解析器构建 DfaBuilder，完成预计算
static std::unique_ptr<DfaBuilder> MakeBuilderFromParser(NFARegexParser& parser) {
    auto builder = std::make_unique<DfaBuilder>(parser.root_node(), parser.node_id_to_token());
    builder->BuildCharClassMap();
    builder->ComputeClassRepresentativeChar();  // 若不存在，可注释掉或实现空函数
    builder->ComputeNfaCharClassMask();
    return builder;  // 移动语义，无需 std::move
}

// 测试最小化 "a*"（应压缩为单状态）
TEST(DfaMinimizerTest, StarA) {
    NFARegexParser parser;
    parser.Register("a*", "star");

    auto builder = MakeBuilderFromParser(parser);
    Dfa raw = builder->BuildDfaFromNfa();
    auto [states, start_state] = builder->MinimizeDfa(raw);

    EXPECT_EQ(states.size(), 1);
    EXPECT_EQ(start_state, 0);

    auto* state = states[0].get();
    EXPECT_TRUE(state->is_accepted);
    EXPECT_EQ(state->token, "star");

    int class_a = builder->char_to_class()['a'];
    EXPECT_EQ(state->class_id_to_next[class_a], 0);
    for (int cid = 0; cid < builder->class_count(); ++cid) {
        if (cid != class_a) {
            EXPECT_EQ(state->class_id_to_next[cid], -1);
        }
    }
}

// 测试最小化 "a|b"（压缩为一个终结结点）
TEST(DfaMinimizerTest, OrAB) {
    NFARegexParser parser;
    parser.Register("a|b", "or");

    auto builder = MakeBuilderFromParser(parser);
    Dfa raw = builder->BuildDfaFromNfa();
    auto [states, start_state] = builder->MinimizeDfa(raw);

    EXPECT_EQ(states.size(), 2);
    EXPECT_EQ(start_state, 0);

    int class_a = builder->char_to_class()['a'];
    int class_b = builder->char_to_class()['b'];

    auto* start = states[0].get();
    EXPECT_NE(start->class_id_to_next[class_a], -1);
    EXPECT_NE(start->class_id_to_next[class_b], -1);
    EXPECT_EQ(start->class_id_to_next[class_a], start->class_id_to_next[class_b]);

    int target_a = start->class_id_to_next[class_a];
    int target_b = start->class_id_to_next[class_b];
    EXPECT_TRUE(states[target_a]->is_accepted);
    EXPECT_TRUE(states[target_b]->is_accepted);
    EXPECT_EQ(states[target_a]->token, "or");
    EXPECT_EQ(states[target_b]->token, "or");

    for (int cid = 0; cid < builder->class_count(); ++cid) {
        EXPECT_EQ(states[target_a]->class_id_to_next[cid], -1);
        EXPECT_EQ(states[target_b]->class_id_to_next[cid], -1);
    }
}

// 测试最小化 "ab"（不应改变）
TEST(DfaMinimizerTest, ConcatAB) {
    NFARegexParser parser;
    parser.Register("ab", "concat");

    auto builder = MakeBuilderFromParser(parser);
    Dfa raw = builder->BuildDfaFromNfa();
    auto [states, start_state] = builder->MinimizeDfa(raw);

    EXPECT_EQ(states.size(), 3);
    EXPECT_EQ(start_state, 0);

    int class_a = builder->char_to_class()['a'];
    int class_b = builder->char_to_class()['b'];

    auto* start = states[0].get();
    EXPECT_NE(start->class_id_to_next[class_a], -1);
    EXPECT_EQ(start->class_id_to_next[class_b], -1);

    int mid_id = start->class_id_to_next[class_a];
    auto* mid = states[mid_id].get();
    EXPECT_NE(mid->class_id_to_next[class_b], -1);
    EXPECT_EQ(mid->class_id_to_next[class_a], -1);

    int accept_id = mid->class_id_to_next[class_b];
    auto* accept = states[accept_id].get();
    EXPECT_TRUE(accept->is_accepted);
    EXPECT_EQ(accept->token, "concat");
    for (int cid = 0; cid < builder->class_count(); ++cid) {
        EXPECT_EQ(accept->class_id_to_next[cid], -1);
    }
}

// 测试最小化 "(a|b)*"（应压缩为单状态）
TEST(DfaMinimizerTest, StarAB) {
    NFARegexParser parser;
    parser.Register("(a|b)*", "star_ab");

    auto builder = MakeBuilderFromParser(parser);
    Dfa raw = builder->BuildDfaFromNfa();
    auto [states, start_state] = builder->MinimizeDfa(raw);

    EXPECT_EQ(states.size(), 1);
    EXPECT_EQ(start_state, 0);

    auto* state = states[0].get();
    EXPECT_TRUE(state->is_accepted);
    EXPECT_EQ(state->token, "star_ab");

    int class_a = builder->char_to_class()['a'];
    int class_b = builder->char_to_class()['b'];
    EXPECT_EQ(state->class_id_to_next[class_a], 0);
    EXPECT_EQ(state->class_id_to_next[class_b], 0);

    for (int cid = 0; cid < builder->class_count(); ++cid) {
        if (cid != class_a && cid != class_b) {
            EXPECT_EQ(state->class_id_to_next[cid], -1);
        }
    }
}

// 测试最小化 "a*|b*"（预期至少2个状态，且起始对a和b的转移不同）
TEST(DfaMinimizerTest, StarAOrStarB) {
    NFARegexParser parser;
    parser.Register("a*|b*", "star_or");

    auto builder = MakeBuilderFromParser(parser);
    Dfa raw = builder->BuildDfaFromNfa();
    auto [states, start_state] = builder->MinimizeDfa(raw);

    int raw_count = static_cast<int>(raw.states.size());
    int min_count = static_cast<int>(states.size());
    EXPECT_LT(min_count, raw_count);
    EXPECT_GE(min_count, 2);

    int class_a = builder->char_to_class()['a'];
    int class_b = builder->char_to_class()['b'];
    auto* start = states[start_state].get();
    EXPECT_NE(start->class_id_to_next[class_a], -1);
    EXPECT_NE(start->class_id_to_next[class_b], -1);
    EXPECT_NE(start->class_id_to_next[class_a], start->class_id_to_next[class_b]);

    int target_a = start->class_id_to_next[class_a];
    int target_b = start->class_id_to_next[class_b];
    EXPECT_TRUE(states[target_a]->is_accepted);
    EXPECT_TRUE(states[target_b]->is_accepted);
    EXPECT_EQ(states[target_a]->class_id_to_next[class_a], target_a);
    EXPECT_EQ(states[target_b]->class_id_to_next[class_b], target_b);
}

}  // namespace cc