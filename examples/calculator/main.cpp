//
// Created by PC on 2026/7/5.
//

#include <iostream>

#include "calculator_core.h"
#include "compile/compiler_input.h"

int main() {
    CalculatorParserData cpd;
    CalculatorLexerCore cld;
    common::Lexer lexer(cld);
    common::Parser parser(cpd);
    while (true) {
        std::cout << "> ";
        auto ci = common::CompilerInput::FromConsole();
        try {
            parser.Parse(lexer, *ci);
        } catch (common::CompileError& e) {
            std::cerr << e.FormatErrorMessage() << std::endl;
        }
    }
}
