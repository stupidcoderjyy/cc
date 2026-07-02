//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderLookaheadTest : public testing::Test {
protected:
    static bool SetContains(const SymbolSet& set, const Symbol& sym) { return set.contains(sym); }
};

// 文法: E -> E + T | T, T -> id
// 验证 E -> T· 的前瞻符 = {$, +}
TEST_F(LALRBuilderLookaheadTest, ExpressionReduceLookahead) {
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol T_nt{"T", SymbolType::kNonTerminal};
    Symbol plus{"+", SymbolType::kTerminal};
    Symbol id{"id", SymbolType::kTerminal};

    syntax.AddProduction(E, {E, plus, T_nt});  // prodId=1
    syntax.AddProduction(E, {T_nt});           // prodId=2
    syntax.AddProduction(T_nt, {id});          // prodId=3

    LALRBuilder builder(syntax);
    builder.Build();

    // 找到包含 E -> T· (prodId=2, dotPos=1) 的 LR(0) 状态
    int target_state = -1;
    for (int s = 0; s < static_cast<int>(builder.lr0_states().size()); ++s) {
        if (builder.lr0_states()[s].items.contains({2, 1})) {
            target_state = s;
            break;
        }
    }
    ASSERT_NE(target_state, -1) << "State with E -> T· not found";

    const auto& la = builder.lr0_lookaheads()[target_state].at({2, 1});
    EXPECT_TRUE(SetContains(la, plus));
    EXPECT_TRUE(SetContains(la, syntax.end_symbol()));
    EXPECT_EQ(la.size(), 2);
}

// 验证 ROOT -> ·E 的前瞻符 = {$}
TEST_F(LALRBuilderLookaheadTest, RootItemLookahead) {
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol id{"id", SymbolType::kTerminal};

    syntax.AddProduction(E, {id});

    LALRBuilder builder(syntax);
    builder.Build();

    const auto& la = builder.lr0_lookaheads()[0].at({0, 0});
    EXPECT_TRUE(SetContains(la, syntax.end_symbol()));
    EXPECT_EQ(la.size(), 1);
}

// 文法: A -> B c, B -> d | ε
// 验证 B -> ·d (prodId=2, dotPos=0) 的前瞻符 = {c}
TEST_F(LALRBuilderLookaheadTest, ClosureSpontaneousLookahead) {
    Syntax syntax;
    Symbol A{"A", SymbolType::kNonTerminal};
    Symbol B{"B", SymbolType::kNonTerminal};
    Symbol c{"c", SymbolType::kTerminal};
    Symbol d{"d", SymbolType::kTerminal};

    syntax.AddProduction(A, {B, c});  // prodId=1
    syntax.AddProduction(B, {d});     // prodId=2
    syntax.AddProduction(B, {});      // prodId=3 (epsilon)

    LALRBuilder builder(syntax);
    builder.Build();

    // 状态0应包含 [1,0] A -> ·B c 和闭包项 [2,0] B -> ·d, [3,0] B -> ·
    // [1,0] 的 LA = {$}，B 后面是 c，所以 [2,0] 和 [3,0] 应获得 {c}
    const auto& la_d = builder.lr0_lookaheads()[0].at({2, 0});
    EXPECT_TRUE(SetContains(la_d, c));
    EXPECT_FALSE(SetContains(la_d, syntax.end_symbol()));

    const auto& la_eps = builder.lr0_lookaheads()[0].at({3, 0});
    EXPECT_TRUE(SetContains(la_eps, c));
}

// 可空链传播: A -> B, B -> C, C -> ε
// 验证 B -> ·C 得到 LA = {$}
TEST_F(LALRBuilderLookaheadTest, NullableChainLookahead) {
    Syntax syntax;
    Symbol A{"A", SymbolType::kNonTerminal};
    Symbol B{"B", SymbolType::kNonTerminal};
    Symbol C{"C", SymbolType::kNonTerminal};

    syntax.AddProduction(A, {B});  // prodId=1
    syntax.AddProduction(B, {C});  // prodId=2
    syntax.AddProduction(C, {});   // prodId=3

    LALRBuilder builder(syntax);
    builder.Build();

    // [1,0] A -> ·B, LA={$}, B后面为空→all_nullable，所以LA传播
    const auto& la_bc = builder.lr0_lookaheads()[0].at({2, 0});
    EXPECT_TRUE(SetContains(la_bc, syntax.end_symbol()));

    // [2,0] B -> ·C, LA={$}, C后面为空→all_nullable，所以LA传播
    const auto& la_ce = builder.lr0_lookaheads()[0].at({3, 0});
    EXPECT_TRUE(SetContains(la_ce, syntax.end_symbol()));
}
