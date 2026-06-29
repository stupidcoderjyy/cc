//
// Created by PC on 2026/6/30.
//
#include "syntax/syntax.h"

#include <gtest/gtest.h>

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
    EXPECT_EQ(syntax.productions()[0].head.name, "E");
    EXPECT_EQ(syntax.productions()[0].body.size(), 3);
}

TEST_F(SyntaxTest, StartSymbol) {
    syntax.SetRootSymbol(E);
    EXPECT_EQ(syntax.root_symbol().name, "E");
    EXPECT_EQ(syntax.root_symbol().type, SymbolType::kNonTerminal);
}

TEST_F(SyntaxTest, SymbolPriorityAndAssociativity) {
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

TEST_F(SyntaxTest, ProductionPriorityOverrideOnlyTarget) {
    int prodId1 = syntax.AddProduction(E, {E, plus, T});
    syntax.AddProduction(T, {T, star, F});
    syntax.SetProductionPriority(prodId1, 5);

    const auto& prods = syntax.productions();
    EXPECT_EQ(prods[0].priority, 5);
    // prodId2 should not be affected
    EXPECT_EQ(prods[1].priority, 0);
}

TEST_F(SyntaxTest, FindSymbolNotFound) {
    const auto* found = syntax.FindSymbol("unknown", SymbolType::kTerminal);
    EXPECT_EQ(found, nullptr);
    const auto* foundNt = syntax.FindSymbol("unknown", SymbolType::kNonTerminal);
    EXPECT_EQ(foundNt, nullptr);
}

TEST_F(SyntaxTest, SetSymbolPropertiesUpdate) {
    // SetSymbolProperties should update an already-existing symbol
    Symbol dup{"dup", SymbolType::kTerminal};
    syntax.SetSymbolPriority(dup, 10);
    syntax.SetSymbolAssociativity(dup, Associativity::kRight);

    // Call SetSymbolProperties with new values on the same symbol
    syntax.SetSymbolProperties(dup, 20, Associativity::kLeft);

    const auto* found = syntax.FindSymbol("dup", SymbolType::kTerminal);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->priority, 20);
    EXPECT_EQ(found->assoc, Associativity::kLeft);
}

TEST_F(SyntaxTest, EmptyProduction) {
    int prodId = syntax.AddProduction(E, {});

    EXPECT_EQ(syntax.productions().size(), 1);
    EXPECT_EQ(syntax.productions()[0].id, prodId);
    EXPECT_EQ(syntax.productions()[0].head.name, "E");
    EXPECT_TRUE(syntax.productions()[0].body.empty());
    // Empty rhs represents an epsilon production
    EXPECT_EQ(syntax.productions()[0].body.size(), 0);
}

TEST_F(SyntaxTest, NonTerminalPriority) {
    Symbol stmt{"stmt", SymbolType::kNonTerminal};
    syntax.SetSymbolProperties(stmt, 3, Associativity::kRight);

    const auto* found = syntax.FindSymbol("stmt", SymbolType::kNonTerminal);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->priority, 3);
    EXPECT_EQ(found->assoc, Associativity::kRight);
    EXPECT_EQ(found->type, SymbolType::kNonTerminal);
}

TEST_F(SyntaxTest, EofSymbolFiltered) {
    syntax.SetRootSymbol(E);
    // The end_symbol_ has type kEof and should not appear in terminals or non_terminals
    EXPECT_EQ(syntax.end_symbol().type, SymbolType::kEof);
    const auto* found = syntax.FindSymbol("$", SymbolType::kEof);
    EXPECT_EQ(found, nullptr);
}

TEST_F(SyntaxTest, AddProductionCollectsSymbols) {
    syntax.AddProduction(E, {E, plus, T});
    syntax.AddProduction(T, {id});

    EXPECT_GT(syntax.terminals().size(), 0);
    EXPECT_GT(syntax.non_terminals().size(), 0);
    // All non-terminals used as LHS should appear in non_terminals
    EXPECT_NE(syntax.FindSymbol("E", SymbolType::kNonTerminal), nullptr);
    EXPECT_NE(syntax.FindSymbol("T", SymbolType::kNonTerminal), nullptr);
    // All terminals used in RHS should appear in terminals
    EXPECT_NE(syntax.FindSymbol("+", SymbolType::kTerminal), nullptr);
    EXPECT_NE(syntax.FindSymbol("id", SymbolType::kTerminal), nullptr);
}
