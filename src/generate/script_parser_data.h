//
// Created by PC on 2026/7/2.
//

#ifndef CC_SCRIPT_LEXER_H
#define CC_SCRIPT_LEXER_H

#include "compile/lexer.h"
#include "compile/parser.h"

namespace cc {

using common::LexerDataSupplier;
using common::ParserDataSupplier;
using common::Property;

struct ScriptLexerData : LexerDataSupplier {
    int CharClassCount() override;
    int StatesCount() override;
    int StartState() override;
    void InitAccepted(std::vector<bool>& vec) override;
    void InitGoto(std::vector<std::vector<int>>& vec) override;
    void InitCharToClass(std::vector<int>& vec) override;
    void InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) override;
};

struct ScriptParserData : ParserDataSupplier {
    int TokenMappersCount() const override;
    int NonTerminalSymbolsCount() const override;
    int TerminalSymbolsCount() const override;
    int StatesCount() const override;
    void InitReduceActions(std::vector<common::ReduceFunc>& vec) override;
    void InitGoto(std::vector<std::vector<int>>& vec) override;
    void InitActions(std::vector<std::vector<int>>& vec) override;
    void InitTokenToSymbol(std::vector<int>& vec) override;
    void InitPropertySuppliers(std::vector<common::PropertySupplier>& vec) override;
    void InitSymbolsAndProductions(std::vector<common::Symbol>& symbols,
                                   std::vector<common::Production>& productions) override;

    //root → script
    void ReduceRoot(const std::vector<std::unique_ptr<Property>>& props);
    //script → block
    void ReduceScript0(const std::vector<std::unique_ptr<Property>>& props);
    //script → script block
    void ReduceScript1(const std::vector<std::unique_ptr<Property>>& props);
    // block → content blockEnd
    void ReduceBlock(const std::vector<std::unique_ptr<Property>>& props);
    // content → syntaxBegin syntax
    void ReduceContent0(const std::vector<std::unique_ptr<Property>>& props);
    // content → tokenBegin tokens
    void ReduceContent1(const std::vector<std::unique_ptr<Property>>& props);
    // syntax → syntax production
    void ReduceSyntax0(const std::vector<std::unique_ptr<Property>>& props);
    // syntax → production
    void ReduceSyntax1(const std::vector<std::unique_ptr<Property>>& props);
    // production → head point body ;
    void ReduceProduction(const std::vector<std::unique_ptr<Property>>& props);
    // head → id
    void ReduceHead0(const std::vector<std::unique_ptr<Property>>& props);
    // head → endHead
    void ReduceHead1(const std::vector<std::unique_ptr<Property>>& props);
    // body → slice
    void ReduceBody0(const std::vector<std::unique_ptr<Property>>& props);
    // body → body | slice
    void ReduceBody1(const std::vector<std::unique_ptr<Property>>& props);
    // slice → seq priorityP
    void ReduceSlice(const std::vector<std::unique_ptr<Property>>& props);
    // priorityP → ε
    void ReducePriorityP0(const std::vector<std::unique_ptr<Property>>& props);
    // priorityP → priorityMarkProd
    void ReducePriorityP1(const std::vector<std::unique_ptr<Property>>& props);
    // seq → symbol
    void ReduceSeq0(const std::vector<std::unique_ptr<Property>>& props);
    // seq → seq symbol
    void ReduceSeq1(const std::vector<std::unique_ptr<Property>>& props);
    // symbol → id
    void ReduceSymbol0(const std::vector<std::unique_ptr<Property>>& props);
    // symbol → terminal priorityT
    void ReduceSymbol1(const std::vector<std::unique_ptr<Property>>& props);
    // priorityT → ε
    void ReducePriorityT0(const std::vector<std::unique_ptr<Property>>& props);
    // priorityT → priorityMarkTerminal
    void ReducePriorityT1(const std::vector<std::unique_ptr<Property>>& props);
    // tokens → token
    void ReduceTokens0(const std::vector<std::unique_ptr<Property>>& props);
    // tokens → tokens token
    void ReduceTokens1(const std::vector<std::unique_ptr<Property>>& props);
    // token → id : string ;
    void ReduceToken(const std::vector<std::unique_ptr<Property>>& props);
};

}  // namespace cc

#endif  //CC_SCRIPT_LEXER_H
