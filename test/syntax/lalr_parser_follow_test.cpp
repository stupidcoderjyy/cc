//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderFollowTest : public ::testing::Test {
protected:
    static bool SetContains(const SymbolSet& set, const Symbol& sym) { return set.contains(sym); }
};

// 文法: E -> E + T | T, T -> id
// FOLLOW(E) = {+, $}
// FOLLOW(T) = {+, $}
TEST_F(LALRBuilderFollowTest, ExpressionGrammar) {
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol T_nt{"T", SymbolType::kNonTerminal};
    Symbol plus{"+", SymbolType::kTerminal};
    Symbol id{"id", SymbolType::kTerminal};

    syntax.AddProduction(E, {E, plus, T_nt});
    syntax.AddProduction(E, {T_nt});
    syntax.AddProduction(T_nt, {id});

    LALRBuilder builder(syntax);

    int eId = builder.IdOf(E);
    int tId = builder.IdOf(T_nt);

    // FOLLOW(E) = {+, $}
    EXPECT_TRUE(SetContains(builder.Follow(eId), plus));
    EXPECT_TRUE(SetContains(builder.Follow(eId), syntax.end_symbol()));
    EXPECT_EQ(builder.Follow(eId).size(), 2);

    // FOLLOW(T) = {+, $}
    EXPECT_TRUE(SetContains(builder.Follow(tId), plus));
    EXPECT_TRUE(SetContains(builder.Follow(tId), syntax.end_symbol()));
    EXPECT_EQ(builder.Follow(tId).size(), 2);
}

// 文法: A -> B c, B -> d | ε
// FOLLOW(A) = {$}
// FOLLOW(B) = {c}
TEST_F(LALRBuilderFollowTest, BasicFollow) {
    Syntax syntax;
    Symbol A{"A", SymbolType::kNonTerminal};
    Symbol B{"B", SymbolType::kNonTerminal};
    Symbol c{"c", SymbolType::kTerminal};
    Symbol d{"d", SymbolType::kTerminal};

    syntax.AddProduction(A, {B, c});
    syntax.AddProduction(B, {d});
    syntax.AddProduction(B, {});

    LALRBuilder builder(syntax);

    int aId = builder.IdOf(A);
    int bId = builder.IdOf(B);

    EXPECT_TRUE(SetContains(builder.Follow(aId), syntax.end_symbol()));
    EXPECT_TRUE(SetContains(builder.Follow(bId), c));
    EXPECT_EQ(builder.Follow(bId).size(), 1);
}

// 文法: A -> B, B -> A (互相递归)
// FOLLOW(A) = {$}, FOLLOW(B) = {$}
TEST_F(LALRBuilderFollowTest, MutualRecursionFollow) {
    Syntax syntax;
    Symbol A{"A", SymbolType::kNonTerminal};
    Symbol B{"B", SymbolType::kNonTerminal};

    syntax.AddProduction(A, {B});
    syntax.AddProduction(B, {A});

    LALRBuilder builder(syntax);

    int aId = builder.IdOf(A);
    int bId = builder.IdOf(B);

    EXPECT_TRUE(SetContains(builder.Follow(aId), syntax.end_symbol()));
    EXPECT_TRUE(SetContains(builder.Follow(bId), syntax.end_symbol()));
}

// 文法: A -> B C, B -> ε, C -> ε
// FOLLOW(A) = {$}, FOLLOW(B) = FOLLOW(C) = {$}
TEST_F(LALRBuilderFollowTest, AllNullable) {
    Syntax syntax;
    Symbol A{"A", SymbolType::kNonTerminal};
    Symbol B{"B", SymbolType::kNonTerminal};
    Symbol C{"C", SymbolType::kNonTerminal};

    syntax.AddProduction(A, {B, C});
    syntax.AddProduction(B, {});
    syntax.AddProduction(C, {});

    LALRBuilder builder(syntax);

    int aId = builder.IdOf(A);
    int bId = builder.IdOf(B);
    int cId = builder.IdOf(C);

    EXPECT_TRUE(SetContains(builder.Follow(aId), syntax.end_symbol()));
    EXPECT_TRUE(SetContains(builder.Follow(bId), syntax.end_symbol()));
    EXPECT_TRUE(SetContains(builder.Follow(cId), syntax.end_symbol()));
}

// 终结符没有 FOLLOW 集
TEST_F(LALRBuilderFollowTest, TerminalHasEmptyFollow) {
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol id{"id", SymbolType::kTerminal};

    syntax.AddProduction(E, {id});

    LALRBuilder builder(syntax);
    int idId = builder.IdOf(id);
    EXPECT_TRUE(builder.Follow(idId).empty());
}
