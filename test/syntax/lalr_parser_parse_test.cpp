//
// Created by PC on 2026/6/30.
//
#include <gtest/gtest.h>

#include "syntax/lalr_parser.h"

using namespace cc;

class LALRBuilderParseTest : public testing::Test {
protected:
    // E -> E + T | T, T -> id
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

// id 单独匹配
TEST_F(LALRBuilderParseTest, SingleToken) {
    LALRBuilder builder(syntax);
    EXPECT_TRUE(builder.DebugParse({id, syntax.end_symbol()}));
}

// id + id
TEST_F(LALRBuilderParseTest, SimpleAddition) {
    LALRBuilder builder(syntax);
    EXPECT_TRUE(builder.DebugParse({id, plus, id, syntax.end_symbol()}));
}

// id + id + id
TEST_F(LALRBuilderParseTest, TwoAdditions) {
    LALRBuilder builder(syntax);
    EXPECT_TRUE(builder.DebugParse({id, plus, id, plus, id, syntax.end_symbol()}));
}

// 语法错误：单独的 +
TEST_F(LALRBuilderParseTest, SyntaxError) {
    LALRBuilder builder(syntax);
    EXPECT_FALSE(builder.DebugParse({plus, id, syntax.end_symbol()}));
}

// 语法错误：空输入
TEST_F(LALRBuilderParseTest, EmptyInputRejected) {
    LALRBuilder builder(syntax);
    // 空输入只有 $，E 不能归约为空
    EXPECT_FALSE(builder.DebugParse({syntax.end_symbol()}));
}

// 重复的 + +
TEST_F(LALRBuilderParseTest, DoublePlus) {
    LALRBuilder builder(syntax);
    EXPECT_FALSE(builder.DebugParse({id, plus, plus, id, syntax.end_symbol()}));
}
