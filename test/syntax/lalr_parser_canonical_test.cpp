//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderCanonicalTest : public ::testing::Test {
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
    // 应产生约 6 个状态（在 5~8 范围内）
    EXPECT_GE(builder.states().size(), 5);
    EXPECT_LE(builder.states().size(), 8);
}

TEST_F(LALRBuilderCanonicalTest, StartStateContainsRootItems) {
    LALRBuilder builder(syntax);

    const auto& s0 = builder.states()[0];
    // ROOT -> ·E
    EXPECT_TRUE(ItemsContain(s0.items, 0, 0));
    // E -> ·E + T
    EXPECT_TRUE(ItemsContain(s0.items, 1, 0));
    // E -> ·T
    EXPECT_TRUE(ItemsContain(s0.items, 2, 0));
    // T -> ·id
    EXPECT_TRUE(ItemsContain(s0.items, 3, 0));
}

TEST_F(LALRBuilderCanonicalTest, StartStateTransitions) {
    LALRBuilder builder(syntax);

    const auto& s0 = builder.states()[0];
    int eId = builder.IdOf(E);
    int tId = builder.IdOf(T_nt);
    int idId = builder.IdOf(id);

    ASSERT_TRUE(s0.transitions.contains(eId));
    int stateE = s0.transitions.at(eId);
    EXPECT_GE(stateE, 0);
    EXPECT_LT(stateE, static_cast<int>(builder.states().size()));

    ASSERT_TRUE(s0.transitions.contains(tId));
    int stateT = s0.transitions.at(tId);
    EXPECT_NE(stateT, stateE);

    ASSERT_TRUE(s0.transitions.contains(idId));
    int stateId = s0.transitions.at(idId);
    EXPECT_NE(stateId, stateE);
    EXPECT_NE(stateId, stateT);
}

TEST_F(LALRBuilderCanonicalTest, StateAfterEHasPlusTransition) {
    LALRBuilder builder(syntax);

    int eId = builder.IdOf(E);
    int plusId = builder.IdOf(plus);

    const auto& s0 = builder.states()[0];
    ASSERT_TRUE(s0.transitions.contains(eId));
    int s1 = s0.transitions.at(eId);

    const auto& stateE = builder.states()[s1];
    EXPECT_TRUE(ItemsContain(stateE.items, 0, 1));  // ROOT -> E·
    EXPECT_TRUE(ItemsContain(stateE.items, 1, 1));  // E -> E·+T
    ASSERT_TRUE(stateE.transitions.contains(plusId));
}

TEST_F(LALRBuilderCanonicalTest, ReduceStatesHaveNoTransitions) {
    LALRBuilder builder(syntax);

    int tId = builder.IdOf(T_nt);
    const auto& [items, transitions] = builder.states()[0];
    ASSERT_TRUE(transitions.contains(tId));
    int stateT = transitions.at(tId);

    const auto& tState = builder.states()[stateT];
    EXPECT_TRUE(ItemsContain(tState.items, 2, 1));  // E -> T·
    EXPECT_TRUE(tState.transitions.empty());
}

TEST_F(LALRBuilderCanonicalTest, NoDuplicateStates) {
    LALRBuilder builder(syntax);

    for (size_t i = 0; i < builder.states().size(); ++i) {
        for (size_t j = i + 1; j < builder.states().size(); ++j) {
            EXPECT_NE(builder.states()[i].items, builder.states()[j].items)
                << "State " << i << " and " << j << " have identical item sets";
        }
    }
}
