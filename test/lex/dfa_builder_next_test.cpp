//
// Created by PC on 2026/6/29.
//
#include <gtest/gtest.h>

#include "lex/dfa_builder.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

// 辅助：从解析器构建 DfaBuilder（假设只注册了一个正则）
std::unique_ptr<DfaBuilder> MakeBuilderFromParser(NFARegexParser& parser) {
    auto builder = std::make_unique<DfaBuilder>(parser.root_node(), parser.node_id_to_token());
    builder->BuildCharClassMap();
    builder->ComputeNfaCharClassMask();
    return std::move(builder);
}

// 测试 Next 对单字符 'a' 的 NFA
TEST(DfaBuilderNextTest, SingleCharA) {
    NFARegexParser parser;
    parser.Register("a", "token_a");  // 正则 "a"，标记为 "token_a"

    auto builder = MakeBuilderFromParser(parser);

    // 获取根节点 ID，构造起始 NFA 集合
    NfaGroup start_group;
    start_group.set(parser.root_node()->id());

    // 计算 ε-闭包（根节点无 ε 边，闭包等于自己）
    auto closure = builder->EpsilonClosure(start_group);

    // 获取字符类 ID：只有一个谓词，所有满足 'a' 的字符归为 class 0
    int class_id = builder->char_to_class()['a'];
    EXPECT_EQ(builder->class_count(), 2);

    // 调用 Next
    auto next_group = builder->Next(class_id, closure);

    // 预期结果：应包含接受节点（即正则 "a" 的结束节点）
    // parser 的节点中，接受节点是最后设置的结束节点，我们可以从 parser.node_id_to_token() 找到接受节点 ID
    // 但更直接：调用 Next 后，结果集应包含接受节点，我们遍历所有节点找到接受标记
    bool found_accept = false;
    for (const auto& nodes = parser.nodes(); const auto& node : nodes) {
        if (node->accepted()) {
            if (next_group.test(node->id())) {
                found_accept = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found_accept) << "Next did not reach accept node for 'a'";
    // 也检查起始节点不在结果中（因为无 ε 边）
    EXPECT_FALSE(next_group.test(parser.root_node()->id()));
}

// 测试 Next 对含 ε 边的 NFA（如 "a*"）
TEST(DfaBuilderNextTest, StarWithEpsilon) {
    NFARegexParser parser;
    parser.Register("a*", "token_star");

    auto builder = MakeBuilderFromParser(parser);

    NfaGroup start_group;
    start_group.set(parser.root_node()->id());
    auto closure = builder->EpsilonClosure(start_group);

    // 由于 "a*" 的起始节点有 ε 边指向自身和接受节点，闭包应包含多个节点
    // 获取字符类
    int class_a = builder->char_to_class()['a'];
    auto next_group = builder->Next(class_a, closure);

    // 预期：Next 后应包含接受节点（因为 a* 允许至少一个 a）
    bool found_accept = false;
    for (const auto& node : parser.nodes()) {
        if (node->accepted() && next_group.test(node->id())) {
            found_accept = true;
            break;
        }
    }
    EXPECT_TRUE(found_accept);

    // 再对非 'a' 字符（如 'b'）测试，应得到空集（因为 b 不匹配 a*）
    int class_b = builder->char_to_class()['b'];  // b 属于其他类（未匹配任何谓词）
    auto next_b = builder->Next(class_b, closure);
    EXPECT_TRUE(next_b.none());  // 应为空
}

// 测试 Next 对 "a|b" 的 NFA，验证不同字符类产生不同目标
TEST(DfaBuilderNextTest, OrAB) {
    NFARegexParser parser;
    parser.Register("a|b", "token_or");

    auto builder = MakeBuilderFromParser(parser);

    NfaGroup start_group;
    start_group.set(parser.root_node()->id());
    auto closure = builder->EpsilonClosure(start_group);

    // 获取字符类
    int class_a = builder->char_to_class()['a'];
    int class_b = builder->char_to_class()['b'];

    auto next_a = builder->Next(class_a, closure);
    auto next_b = builder->Next(class_b, closure);

    // 两个结果都非空，且应不相同（指向不同的接受节点）
    EXPECT_FALSE(next_a.none());
    EXPECT_FALSE(next_b.none());
    EXPECT_NE(next_a, next_b);  // 比较 bitset 内容不同

    // 检查每个结果分别包含对应的接受节点
    // 由于 parser 中接受节点可能有多个，但每个分支各有一个，我们检查存在性
    bool found_a = false, found_b = false;
    for (const auto& node : parser.nodes()) {
        if (node->accepted()) {
            if (next_a.test(node->id())) found_a = true;
            if (next_b.test(node->id())) found_b = true;
        }
    }
    EXPECT_TRUE(found_a);
    EXPECT_TRUE(found_b);
}

}  // namespace cc