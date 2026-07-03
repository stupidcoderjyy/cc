//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderCanonicalTest : public testing::Test {
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

TEST_F(LALRBuilderCanonicalTest, StateCount) {
    LALRBuilder builder(syntax);
    builder.Build();
    EXPECT_GE(builder.lr0_states().size(), 5);
    EXPECT_LE(builder.lr0_states().size(), 8);
}

TEST_F(LALRBuilderCanonicalTest, StartStateContainsRootItems) {
    LALRBuilder builder(syntax);
    builder.Build();

    const auto& s0 = builder.lr0_states()[0];
    EXPECT_TRUE(ItemsContain(s0.items, 0, 0));
    EXPECT_TRUE(ItemsContain(s0.items, 1, 0));
    EXPECT_TRUE(ItemsContain(s0.items, 2, 0));
    EXPECT_TRUE(ItemsContain(s0.items, 3, 0));
}

TEST_F(LALRBuilderCanonicalTest, StartStateTransitions) {
    LALRBuilder builder(syntax);
    builder.Build();

    const auto& s0 = builder.lr0_states()[0];
    int eId = builder.IdOf(E);
    int tId = builder.IdOf(T_nt);
    int idId = builder.IdOf(id);

    ASSERT_TRUE(s0.transitions.contains(eId));
    ASSERT_TRUE(s0.transitions.contains(tId));
    ASSERT_TRUE(s0.transitions.contains(idId));
    EXPECT_NE(s0.transitions.at(eId), s0.transitions.at(tId));
    EXPECT_NE(s0.transitions.at(eId), s0.transitions.at(idId));
    EXPECT_NE(s0.transitions.at(tId), s0.transitions.at(idId));
}

TEST_F(LALRBuilderCanonicalTest, StateAfterEHasPlusTransition) {
    LALRBuilder builder(syntax);
    builder.Build();

    int eId = builder.IdOf(E);
    int plusId = builder.IdOf(plus);

    const auto& s0 = builder.lr0_states()[0];
    ASSERT_TRUE(s0.transitions.contains(eId));
    int s1 = s0.transitions.at(eId);

    const auto& stateE = builder.lr0_states()[s1];
    EXPECT_TRUE(ItemsContain(stateE.items, 0, 1));
    EXPECT_TRUE(ItemsContain(stateE.items, 1, 1));
    ASSERT_TRUE(stateE.transitions.contains(plusId));
}

TEST_F(LALRBuilderCanonicalTest, ReduceStatesHaveNoTransitions) {
    LALRBuilder builder(syntax);
    builder.Build();

    int tId = builder.IdOf(T_nt);
    const auto& s0 = builder.lr0_states()[0];
    ASSERT_TRUE(s0.transitions.contains(tId));
    int stateT = s0.transitions.at(tId);

    const auto& tState = builder.lr0_states()[stateT];
    EXPECT_TRUE(ItemsContain(tState.items, 2, 1));
    EXPECT_TRUE(tState.transitions.empty());
}

TEST_F(LALRBuilderCanonicalTest, NoDuplicateStates) {
    LALRBuilder builder(syntax);
    builder.Build();

    for (size_t i = 0; i < builder.lr0_states().size(); ++i) {
        for (size_t j = i + 1; j < builder.lr0_states().size(); ++j) {
            EXPECT_NE(builder.lr0_states()[i].items, builder.lr0_states()[j].items)
                    << "State " << i << " and " << j << " have identical item sets";
        }
    }
}
