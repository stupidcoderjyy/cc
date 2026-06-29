//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderMergeTest : public ::testing::Test {
protected:
    // 文法：0: ROOT -> E, 1: E -> E+T, 2: E -> T, 3: T -> id
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol T_nt{"T", SymbolType::kNonTerminal};
    Symbol plus{"+", SymbolType::kTerminal};
    Symbol id{"id", SymbolType::kTerminal};

    void SetUp() override {
        syntax.AddProduction(E, {E, plus, T_nt});
        syntax.AddProduction(E, {T_nt});
        syntax.AddProduction(T_nt, {id});
    }
};

// 简单表达式文法没有同心状态，合并后状态数不变
TEST_F(LALRBuilderMergeTest, MergePreservesStateCount) {
    LALRBuilder builder(syntax);

    EXPECT_EQ(builder.lalr_states().size(), builder.lr0_states().size());

    // lr0_to_lalr 应为恒等映射
    for (size_t i = 0; i < builder.lr0_states().size(); ++i) {
        EXPECT_EQ(builder.lr0_to_lalr()[i], static_cast<int>(i));
    }
}

// 合并后 LALR 状态的项集与对应 LR(0) 状态相同
TEST_F(LALRBuilderMergeTest, LALRItemsMatchLR0) {
    LALRBuilder builder(syntax);

    for (size_t i = 0; i < builder.lr0_states().size(); ++i) {
        int lalr_id = builder.lr0_to_lalr()[i];
        EXPECT_EQ(builder.lalr_states()[lalr_id].items, builder.lr0_states()[i].items);
    }
}

// 合并后 GOTO 转移保持一致
TEST_F(LALRBuilderMergeTest, LALRTransitionsMatchLR0) {
    LALRBuilder builder(syntax);

    for (size_t i = 0; i < builder.lr0_states().size(); ++i) {
        int lalr_i = builder.lr0_to_lalr()[i];
        const auto& lr0_trans = builder.lr0_states()[i].transitions;
        const auto& lalr_trans = builder.lalr_states()[lalr_i].transitions;

        for (const auto& [sym_id, target] : lr0_trans) {
            ASSERT_TRUE(lalr_trans.contains(sym_id));
            EXPECT_EQ(lalr_trans.at(sym_id), builder.lr0_to_lalr()[target]);
        }
    }
}
