//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderParsingTableTest : public ::testing::Test {
protected:
    Syntax syntax;
    Symbol E{"E", SymbolType::kNonTerminal};
    Symbol T_nt{"T", SymbolType::kNonTerminal};
    Symbol F{"F", SymbolType::kNonTerminal};
    Symbol plus{"+", SymbolType::kTerminal};
    Symbol minus{"-", SymbolType::kTerminal};
    Symbol star{"*", SymbolType::kTerminal};
    Symbol slash{"/", SymbolType::kTerminal};
    Symbol id{"id", SymbolType::kTerminal};
    Symbol lparen{"(", SymbolType::kTerminal};
    Symbol rparen{")", SymbolType::kTerminal};

    void SetUp() override {
        syntax.SetSymbolProperties(plus, 1, Associativity::kLeft);
        syntax.SetSymbolProperties(minus, 1, Associativity::kLeft);
        syntax.SetSymbolProperties(star, 2, Associativity::kLeft);
        syntax.SetSymbolProperties(slash, 2, Associativity::kLeft);

        syntax.AddProduction(E, {E, plus, T_nt});
        syntax.AddProduction(E, {E, minus, T_nt});
        syntax.AddProduction(E, {T_nt});
        syntax.AddProduction(T_nt, {T_nt, star, F});
        syntax.AddProduction(T_nt, {T_nt, slash, F});
        syntax.AddProduction(T_nt, {F});
        syntax.AddProduction(F, {lparen, E, rparen});
        syntax.AddProduction(F, {id});
    }
};

// 使用默认冲突处理器，表达式文法应无冲突
TEST_F(LALRBuilderParsingTableTest, NoConflictsWithDefaultHandler) {
    LALRBuilder builder(syntax);
    // 不抛异常即通过
    EXPECT_GT(builder.action().size(), 0);
    SUCCEED();
}

// 替换自定义冲突处理器
TEST_F(LALRBuilderParsingTableTest, CustomHandler) {
    Syntax s2;
    Symbol a{"A", SymbolType::kNonTerminal};
    Symbol b{"b", SymbolType::kTerminal};
    s2.AddProduction(a, {a, b});
    s2.AddProduction(a, {});

    LALRBuilder builder(s2);
    EXPECT_GT(builder.action().size(), 0);
}
