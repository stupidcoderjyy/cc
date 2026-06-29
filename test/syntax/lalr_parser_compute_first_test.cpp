//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderFirstSetTest : public ::testing::Test {
protected:
    Syntax syntax;
    Symbol A{"A", SymbolType::kNonTerminal};
    Symbol B{"B", SymbolType::kNonTerminal};
    Symbol c{"c", SymbolType::kTerminal};
    Symbol d{"d", SymbolType::kTerminal};

    static bool SetContains(const SymbolSet& set, const Symbol& sym) { return set.contains(sym); }
};

// A -> B c, B -> d | ε
// FIRST(B) = {d}, can_be_empty(B) = true
// FIRST(A) = {d, c}, can_be_empty(A) = false
TEST_F(LALRBuilderFirstSetTest, BasicFirst) {
    syntax.AddProduction(A, {B, c});
    syntax.AddProduction(B, {d});
    syntax.AddProduction(B, {});  // epsilon

    LALRBuilder builder(syntax);

    int idA = builder.IdOf(A);
    int idB = builder.IdOf(B);
    int idC = builder.IdOf(c);
    int idD = builder.IdOf(d);

    // FIRST(B) = {d}
    EXPECT_TRUE(SetContains(builder.First(idB), d));
    EXPECT_FALSE(SetContains(builder.First(idB), c));
    // B can be empty
    EXPECT_TRUE(builder.HasEpsilon(idB));

    // FIRST(A) = {d, c}
    EXPECT_TRUE(SetContains(builder.First(idA), d));
    EXPECT_TRUE(SetContains(builder.First(idA), c));
    // A cannot be empty
    EXPECT_FALSE(builder.HasEpsilon(idA));

    // FIRST(d) = {d}, cannot be empty
    EXPECT_TRUE(SetContains(builder.First(idD), d));
    EXPECT_FALSE(builder.HasEpsilon(idD));

    // FIRST(c) = {c}
    EXPECT_TRUE(SetContains(builder.First(idC), c));
}

// A -> B C D, B -> ε, C -> ε, D -> d
// FIRST(A) = {d}, can_be_empty(A) = false
TEST_F(LALRBuilderFirstSetTest, MultipleNullable) {
    Symbol C{"C", SymbolType::kNonTerminal};
    Symbol D{"D", SymbolType::kNonTerminal};
    syntax.AddProduction(A, {B, C, D});
    syntax.AddProduction(B, {});  // B -> ε
    syntax.AddProduction(C, {});  // C -> ε
    syntax.AddProduction(D, {d});

    LALRBuilder builder(syntax);

    int idA = builder.IdOf(A);
    int idB = builder.IdOf(B);
    int idC = builder.IdOf(C);

    // A can NOT be empty (D is not nullable)
    EXPECT_FALSE(builder.HasEpsilon(idA));
    EXPECT_TRUE(builder.HasEpsilon(idB));
    EXPECT_TRUE(builder.HasEpsilon(idC));

    // FIRST(A) should contain d (from D, through B and C which are nullable)
    EXPECT_TRUE(SetContains(builder.First(idA), d));
}

// A -> ε
// FIRST(A) = {}, can_be_empty(A) = true
TEST_F(LALRBuilderFirstSetTest, PureEpsilon) {
    syntax.AddProduction(A, {});

    LALRBuilder builder(syntax);

    int idA = builder.IdOf(A);
    EXPECT_TRUE(builder.First(idA).empty());
    EXPECT_TRUE(builder.HasEpsilon(idA));
}

// A -> B, B -> A  (mutual recursion, no terminals)
// FIRST(A) = {}, can_be_empty(A) = false (neither derives epsilon)
TEST_F(LALRBuilderFirstSetTest, MutualRecursion) {
    syntax.AddProduction(A, {B});
    syntax.AddProduction(B, {A});

    LALRBuilder builder(syntax);

    int idA = builder.IdOf(A);
    int idB = builder.IdOf(B);
    EXPECT_TRUE(builder.First(idA).empty());
    EXPECT_FALSE(builder.HasEpsilon(idA));
    EXPECT_TRUE(builder.First(idB).empty());
    EXPECT_FALSE(builder.HasEpsilon(idB));
}

// EOF symbol: FIRST($) = {$}, can_be_empty = false
TEST_F(LALRBuilderFirstSetTest, EofSymbol) {
    syntax.AddProduction(A, {c});  // add at least one production to avoid empty grammar
    LALRBuilder builder(syntax);

    int eofId = builder.IdOf(syntax.end_symbol());
    EXPECT_TRUE(SetContains(builder.First(eofId), syntax.end_symbol()));
    EXPECT_FALSE(builder.HasEpsilon(eofId));
}
