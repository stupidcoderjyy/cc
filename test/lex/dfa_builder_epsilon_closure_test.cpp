//
// Created by PC on 2026/6/28.
//
#include <gtest/gtest.h>

#include "lex/dfa_builder.h"
#include "lex/nfa_node.h"

namespace cc {

class DfaBuilderEpsilonClosureTest : public testing::Test {
protected:
    void TearDown() override {
        // 测试中手动 new 的节点未统一管理，为简化未做清理。
        // 实际项目中建议使用智能指针或记录分配指针。
    }
};

// 1. 单节点无 ε 边
TEST_F(DfaBuilderEpsilonClosureTest, EpsilonClosureSingleNode) {
    auto* n0 = new NFANode();
    n0->set_id(0);

    // 根节点就是 n0
    DFABuilder builder(n0, {});

    NFAGroup group;
    group.set(n0->id());
    auto closure = builder.EpsilonClosure(group);

    EXPECT_TRUE(closure.test(n0->id()));
    EXPECT_EQ(closure.count(), 1);

    delete n0;
}

// 2. ε 链：0 -> 1 -> 2
TEST_F(DfaBuilderEpsilonClosureTest, EpsilonClosureChain) {
    auto* n0 = new NFANode();
    auto* n1 = new NFANode();
    auto* n2 = new NFANode();
    n0->set_id(0);
    n1->set_id(1);
    n2->set_id(2);

    n0->AddEpsilonEdge(n1);
    n1->AddEpsilonEdge(n2);

    DFABuilder builder(n0, {});  // n0 为根，n1/n2 通过 ε 边可达

    NFAGroup group;
    group.set(n0->id());
    auto closure = builder.EpsilonClosure(group);

    EXPECT_TRUE(closure.test(n0->id()));
    EXPECT_TRUE(closure.test(n1->id()));
    EXPECT_TRUE(closure.test(n2->id()));
    EXPECT_EQ(closure.count(), 3);

    delete n0;
    delete n1;
    delete n2;
}

// 3. ε 循环：0 <-> 1
TEST_F(DfaBuilderEpsilonClosureTest, EpsilonClosureCycle) {
    auto* n0 = new NFANode();
    auto* n1 = new NFANode();
    n0->set_id(0);
    n1->set_id(1);

    n0->AddEpsilonEdge(n1);
    n1->AddEpsilonEdge(n0);

    DFABuilder builder(n0, {});

    NFAGroup group;
    group.set(n0->id());
    auto closure = builder.EpsilonClosure(group);

    EXPECT_TRUE(closure.test(n0->id()));
    EXPECT_TRUE(closure.test(n1->id()));
    EXPECT_EQ(closure.count(), 2);

    delete n0;
    delete n1;
}

// 4. 字符边被忽略
TEST_F(DfaBuilderEpsilonClosureTest, EpsilonClosureIgnoreCharEdge) {
    auto* n0 = new NFANode();
    auto* n1 = new NFANode();
    n0->set_id(0);
    n1->set_id(1);

    // 字符边：从 n0 到 n1
    n0->AddCharEdge([](int) { return true; }, n1);

    DFABuilder builder(n0, {});  // n1 通过字符边可达，但 ε 闭包应忽略

    NFAGroup group;
    group.set(n0->id());
    auto closure = builder.EpsilonClosure(group);

    EXPECT_TRUE(closure.test(n0->id()));
    EXPECT_FALSE(closure.test(n1->id()));  // 字符边不应被 ε 闭包包含
    EXPECT_EQ(closure.count(), 1);

    delete n0;
    delete n1;
}

// 5. 多起始状态集合（通过虚拟根节点连接）
TEST_F(DfaBuilderEpsilonClosureTest, EpsilonClosureMultipleStart) {
    // 创建虚拟根节点（ID 为 99），通过 ε 边连接到 n0 和 n1
    auto* root = new NFANode();
    root->set_id(99);

    auto* n0 = new NFANode();
    auto* n1 = new NFANode();
    auto* n2 = new NFANode();
    auto* n3 = new NFANode();

    n0->set_id(0);
    n1->set_id(1);
    n2->set_id(2);
    n3->set_id(3);

    // n0 -> n2, n1 -> n3
    n0->AddEpsilonEdge(n2);
    n1->AddEpsilonEdge(n3);

    // 虚拟根通过 ε 边到 n0 和 n1
    root->AddEpsilonEdge(n0, n1);  // 使用双 ε 边

    DFABuilder builder(root, {});  // 所有节点从 root 可达

    NFAGroup group;
    group.set(n0->id());
    group.set(n1->id());
    auto closure = builder.EpsilonClosure(group);

    EXPECT_TRUE(closure.test(n0->id()));
    EXPECT_TRUE(closure.test(n1->id()));
    EXPECT_TRUE(closure.test(n2->id()));
    EXPECT_TRUE(closure.test(n3->id()));
    EXPECT_EQ(closure.count(), 4);

    delete root;
    delete n0;
    delete n1;
    delete n2;
    delete n3;
}

// 6. 缓存验证（两次调用返回相同结果，并可能命中缓存）
TEST_F(DfaBuilderEpsilonClosureTest, EpsilonClosureCache) {
    auto* n0 = new NFANode();
    auto* n1 = new NFANode();
    n0->set_id(0);
    n1->set_id(1);

    n0->AddEpsilonEdge(n1);

    DFABuilder builder(n0, {});

    NFAGroup group;
    group.set(n0->id());

    auto closure1 = builder.EpsilonClosure(group);
    auto closure2 = builder.EpsilonClosure(group);  // 应命中缓存

    EXPECT_EQ(closure1, closure2);

    delete n0;
    delete n1;
}

}  // namespace cc