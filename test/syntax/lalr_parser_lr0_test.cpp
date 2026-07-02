//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderLR0Test : public testing::Test {
protected:
    // 文法：
    // 0: ROOT -> E    (自动插入)
    // 1: E -> E + T
    // 2: E -> T
    // 3: T -> id
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol T_nt{"T", SymbolType::kNonTerminal};
    Symbol plus{"+", SymbolType::kTerminal};
    Symbol id{"id", SymbolType::kTerminal};

    void SetUp() override {
        syntax.AddProduction(E, {E, plus, T_nt});  // prodId=1
        syntax.AddProduction(E, {T_nt});           // prodId=2
        syntax.AddProduction(T_nt, {id});          // prodId=3
    }

    static bool ItemsContain(const std::set<Item>& items, int prodId, int dotPos) {
        return items.contains({prodId, dotPos});
    }
};

// Closure：{ROOT -> ·E} 应扩展出 E 和 T 的所有产生式
TEST_F(LALRBuilderLR0Test, ClosureFromRootStart) {
    LALRBuilder builder(syntax);

    std::set<Item> initial = {{0, 0}};  // ROOT -> ·E
    auto closed = builder.Closure(initial);

    // ROOT -> ·E
    EXPECT_TRUE(ItemsContain(closed, 0, 0));
    // E -> ·E + T (prodId=1)
    EXPECT_TRUE(ItemsContain(closed, 1, 0));
    // E -> ·T (prodId=2)
    EXPECT_TRUE(ItemsContain(closed, 2, 0));
    // T -> ·id (prodId=3)
    EXPECT_TRUE(ItemsContain(closed, 3, 0));
    // 不应包含圆点位移后的项
    EXPECT_FALSE(ItemsContain(closed, 1, 1));
    EXPECT_FALSE(ItemsContain(closed, 2, 1));
}

// Closure 对已闭合集合为幂等
TEST_F(LALRBuilderLR0Test, ClosureIdempotent) {
    LALRBuilder builder(syntax);

    std::set<Item> initial = {{0, 0}};
    auto closed1 = builder.Closure(initial);
    auto closed2 = builder.Closure(closed1);

    EXPECT_EQ(closed1, closed2);
}

// GOTO：从闭包沿 E 移动圆点 → 得 {ROOT -> E·, E -> E·+T}
TEST_F(LALRBuilderLR0Test, GotoOnE) {
    LALRBuilder builder(syntax);

    std::set<Item> initial = {{0, 0}};
    auto closed = builder.Closure(initial);
    int eId = builder.IdOf(E);
    auto gotoResult = builder.GotoFunc(closed, eId);

    // ROOT -> E· (id=0, dot at pos 1)
    EXPECT_TRUE(ItemsContain(gotoResult, 0, 1));
    // E -> E·+T (id=1, dot at pos 1)
    EXPECT_TRUE(ItemsContain(gotoResult, 1, 1));
    // 不应包含其他位移后的项
    EXPECT_FALSE(ItemsContain(gotoResult, 2, 1));
}

// GOTO：沿终结符 + 移动——闭包中无 + 在圆点后
TEST_F(LALRBuilderLR0Test, GotoOnPlusEmpty) {
    LALRBuilder builder(syntax);

    std::set<Item> initial = {{0, 0}};
    auto closed = builder.Closure(initial);
    int plusId = builder.IdOf(plus);
    auto result = builder.GotoFunc(closed, plusId);

    EXPECT_TRUE(result.empty());
}

// GOTO：沿 T 移动 → 得 {E -> T·}（规约项）
TEST_F(LALRBuilderLR0Test, GotoOnT) {
    LALRBuilder builder(syntax);

    std::set<Item> initial = {{0, 0}};
    auto closed = builder.Closure(initial);
    int tId = builder.IdOf(T_nt);
    auto gotoResult = builder.GotoFunc(closed, tId);

    // E -> T· (id=2, dot at pos 1)
    EXPECT_TRUE(ItemsContain(gotoResult, 2, 1));
    // 不应包含 T -> ·id (GOTO沿T之前已被闭包处理)
    EXPECT_FALSE(ItemsContain(gotoResult, 3, 0));
}

// GOTO 沿 EOF——得空集
TEST_F(LALRBuilderLR0Test, GotoOnEofEmpty) {
    LALRBuilder builder(syntax);

    std::set<Item> initial = {{0, 0}};
    auto closed = builder.Closure(initial);
    int eofId = builder.IdOf(syntax.end_symbol());
    auto result = builder.GotoFunc(closed, eofId);

    EXPECT_TRUE(result.empty());
}
