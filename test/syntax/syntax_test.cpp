//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>
#include "syntax/syntax.h"

using namespace cc;

class SyntaxTest : public ::testing::Test {
protected:
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol T{"T", SymbolType::kNonTerminal};
    Symbol F{"F", SymbolType::kNonTerminal};
    Symbol plus{"+", SymbolType::kTerminal};
    Symbol star{"*", SymbolType::kTerminal};
    Symbol id{"id", SymbolType::kTerminal};
    Symbol lparen{"(", SymbolType::kTerminal};
    Symbol rparen{")", SymbolType::kTerminal};
};

TEST_F(SyntaxTest, AddProductionAndRetrieve) {
    int id1 = syntax.AddProduction(E, {E, plus, T});
    syntax.AddProduction(E, {T});
    syntax.AddProduction(T, {T, star, F});
    syntax.AddProduction(T, {F});
    syntax.AddProduction(F, {lparen, E, rparen});
    syntax.AddProduction(F, {id});

    EXPECT_EQ(syntax.productions().size(), 6);
    EXPECT_EQ(syntax.productions()[0].id, id1);
    EXPECT_EQ(syntax.productions()[0].lhs.name, "E");
    EXPECT_EQ(syntax.productions()[0].rhs.size(), 3);
}

TEST_F(SyntaxTest, StartSymbol) {
    syntax.SetStartSymbol(E);
    EXPECT_EQ(syntax.start_symbol().name, "E");
    EXPECT_EQ(syntax.start_symbol().type, SymbolType::kNonTerminal);
}

TEST_F(SyntaxTest, SymbolPriorityAndAssociativity) {
    // 设置终结符优先级
    syntax.SetSymbolProperties(plus, 1, Associativity::kLeft);
    syntax.SetSymbolProperties(star, 2, Associativity::kLeft);

    const auto* foundPlus = syntax.FindSymbol("+", SymbolType::kTerminal);
    ASSERT_NE(foundPlus, nullptr);
    EXPECT_EQ(foundPlus->priority, 1);
    EXPECT_EQ(foundPlus->assoc, Associativity::kLeft);

    const auto* foundStar = syntax.FindSymbol("*", SymbolType::kTerminal);
    ASSERT_NE(foundStar, nullptr);
    EXPECT_EQ(foundStar->priority, 2);
}

TEST_F(SyntaxTest, ProductionPriorityOverride) {
    int prodId = syntax.AddProduction(E, {E, plus, T});
    syntax.SetProductionPriority(prodId, 5);
    syntax.SetProductionAssociativity(prodId, Associativity::kRight);

    const auto& prods = syntax.productions();
    ASSERT_FALSE(prods.empty());
    EXPECT_EQ(prods[0].priority, 5);
    EXPECT_EQ(prods[0].assoc, Associativity::kRight);
}

TEST_F(SyntaxTest, FindSymbolNotFound) {
    const auto* found = syntax.FindSymbol("unknown", SymbolType::kTerminal);
    EXPECT_EQ(found, nullptr);
}

TEST_F(SyntaxTest, AddSymbolDuplicate) {
    Symbol s{"dup", SymbolType::kTerminal};
    syntax.SetSymbolPriority(s, 10);
    syntax.SetSymbolAssociativity(s, Associativity::kRight);
    // 再次添加相同符号，属性应保持不变（或覆盖？根据设计是忽略新属性）
    Symbol s2{"dup", SymbolType::kTerminal, 20, Associativity::kLeft};
    syntax.SetSymbolProperties(s2, 20, Associativity::kLeft); // 应更新为20
    const auto* found = syntax.FindSymbol("dup", SymbolType::kTerminal);
    ASSERT_NE(found, nullptr);
    // 因为 SetSymbolProperties 会更新已有符号，所以变为20
    EXPECT_EQ(found->priority, 20);
    EXPECT_EQ(found->assoc, Associativity::kLeft);
}