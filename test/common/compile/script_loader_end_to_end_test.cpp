//
// Created by PC on 2026/7/2.
//

#include <gtest/gtest.h>

#include <fstream>

#include "compile/compiler_input.h"
#include "compile/lexer.h"
#include "compile/parser.h"
#include "generate/script_parser_data.h"

namespace cc::gen {

using common::CompilerInput;
using common::Lexer;
using common::Parser;

class ScriptLoaderEndToEndTest : public testing::Test {
protected:
    ScriptLexerData lexer_data_;
    ScriptParserData parser_data_;
};

// Parse lang.txt (empty token/syntax blocks) using scriptloader grammar
TEST_F(ScriptLoaderEndToEndTest, LexerTest) {
    auto ci = CompilerInput::FromString(
            R"(%%TOKEN %%SYNTAX %% a a1 a_ "d \\ \"a" @a @$a @~ '\n' 'a' %12L %2 %R $12L $2 $R)");
    std::vector expected_types{
            kTokenTokenBegin, kTokenSyntaxBegin, kTokenBlockEnd, kTokenId, kTokenId, kTokenId,
            kTokenString, kTokenTerminal, kTokenTerminal, kTokenTerminal, kTokenTerminal,
            kTokenTerminal, kTokenProdMark, kTokenProdMark, kTokenProdMark, kTokenSymbMark,
            kTokenSymbMark, kTokenSymbMark,
            0  //EOF
    };
    Lexer lexer(lexer_data_);

    for (int expected_type : expected_types) {
        auto token = lexer.NextToken(*ci);
        ASSERT_TRUE(token.get());
        EXPECT_EQ(token->Type(), expected_type);
    }
}

// Parse scriptloader.txt using its own grammar
TEST_F(ScriptLoaderEndToEndTest, ParseScriptLoaderTxt) {
    auto ci = CompilerInput::FromFile("lang.txt");

    Lexer lexer(lexer_data_);
    Parser parser(parser_data_);

    try {
        parser.Parse(lexer, *ci);
    } catch (common::CompileError& e) {
        std::cerr << e.FormatErrorMessage() << std::endl;
        FAIL();
    }
    SUCCEED();
}

}  // namespace cc::gen
