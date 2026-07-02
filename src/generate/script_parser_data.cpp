//
// Created by PC on 2026/7/2.
//

#include "script_parser_data.h"

namespace cc {
int ScriptLexerData::CharClassCount() {}

int ScriptLexerData::StatesCount() {}

int ScriptLexerData::StartState() {}

void ScriptLexerData::InitAccepted(std::vector<bool>& vec) {}

void ScriptLexerData::InitGoto(std::vector<std::vector<int>>& vec) {}

void ScriptLexerData::InitCharToClass(std::vector<int>& vec) {}

void ScriptLexerData::InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) {}

int ScriptParserData::TokenMappersCount() const {}

int ScriptParserData::NonTerminalSymbolsCount() const {}

int ScriptParserData::TerminalSymbolsCount() const {}

int ScriptParserData::StatesCount() const {}

void ScriptParserData::InitReduceActions(std::vector<common::ReduceFunc>& vec) {
    vec[0] = [this](auto& props) { ReduceRoot(props); };
    vec[1] = [this](auto& props) { ReduceScript0(props); };
    vec[2] = [this](auto& props) { ReduceScript1(props); };
    vec[3] = [this](auto& props) { ReduceBlock(props); };
    vec[4] = [this](auto& props) { ReduceContent0(props); };
    vec[5] = [this](auto& props) { ReduceContent1(props); };
    vec[6] = [this](auto& props) { ReduceSyntax0(props); };
    vec[7] = [this](auto& props) { ReduceSyntax1(props); };
    vec[8] = [this](auto& props) { ReduceProduction(props); };
    vec[9] = [this](auto& props) { ReduceHead0(props); };
    vec[10] = [this](auto& props) { ReduceHead1(props); };
    vec[11] = [this](auto& props) { ReduceBody0(props); };
    vec[12] = [this](auto& props) { ReduceBody1(props); };
    vec[13] = [this](auto& props) { ReduceSlice(props); };
    vec[14] = [this](auto& props) { ReducePriorityP0(props); };
    vec[15] = [this](auto& props) { ReducePriorityP1(props); };
    vec[16] = [this](auto& props) { ReduceSeq0(props); };
    vec[17] = [this](auto& props) { ReduceSeq1(props); };
    vec[18] = [this](auto& props) { ReduceSymbol0(props); };
    vec[19] = [this](auto& props) { ReduceSymbol1(props); };
    vec[20] = [this](auto& props) { ReducePriorityT0(props); };
    vec[21] = [this](auto& props) { ReducePriorityT1(props); };
    vec[22] = [this](auto& props) { ReduceTokens0(props); };
    vec[23] = [this](auto& props) { ReduceTokens1(props); };
    vec[24] = [this](auto& props) { ReduceToken(props); };
}

void ScriptParserData::InitGoto(std::vector<std::vector<int>>& vec) {}

void ScriptParserData::InitActions(std::vector<std::vector<int>>& vec) {}

void ScriptParserData::InitTokenToSymbol(std::vector<int>& vec) {}

void ScriptParserData::InitPropertySuppliers(std::vector<common::PropertySupplier>& vec) {}

void ScriptParserData::InitSymbolsAndProductions(std::vector<common::Symbol>& symbols,
                                                 std::vector<common::Production>& productions) {}

// root → script
void ScriptParserData::ReduceRoot(const std::vector<std::unique_ptr<Property>>& props) {}

// script → block
void ScriptParserData::ReduceScript0(const std::vector<std::unique_ptr<Property>>& props) {}

// script → script block
void ScriptParserData::ReduceScript1(const std::vector<std::unique_ptr<Property>>& props) {}

// block → content blockEnd
void ScriptParserData::ReduceBlock(const std::vector<std::unique_ptr<Property>>& props) {}

// content → syntaxBegin syntax
void ScriptParserData::ReduceContent0(const std::vector<std::unique_ptr<Property>>& props) {}

// content → tokenBegin tokens
void ScriptParserData::ReduceContent1(const std::vector<std::unique_ptr<Property>>& props) {}

// syntax → syntax production
void ScriptParserData::ReduceSyntax0(const std::vector<std::unique_ptr<Property>>& props) {}

// syntax → production
void ScriptParserData::ReduceSyntax1(const std::vector<std::unique_ptr<Property>>& props) {}

// production → head point body ;
void ScriptParserData::ReduceProduction(const std::vector<std::unique_ptr<Property>>& props) {}

// head → id
void ScriptParserData::ReduceHead0(const std::vector<std::unique_ptr<Property>>& props) {}

// head → endHead
void ScriptParserData::ReduceHead1(const std::vector<std::unique_ptr<Property>>& props) {}

// body → slice
void ScriptParserData::ReduceBody0(const std::vector<std::unique_ptr<Property>>& props) {}

// body → body | slice
void ScriptParserData::ReduceBody1(const std::vector<std::unique_ptr<Property>>& props) {}

// slice → seq priorityP
void ScriptParserData::ReduceSlice(const std::vector<std::unique_ptr<Property>>& props) {}

// priorityP → ε
void ScriptParserData::ReducePriorityP0(const std::vector<std::unique_ptr<Property>>& props) {}

// priorityP → priorityMarkProd
void ScriptParserData::ReducePriorityP1(const std::vector<std::unique_ptr<Property>>& props) {}

// seq → symbol
void ScriptParserData::ReduceSeq0(const std::vector<std::unique_ptr<Property>>& props) {}

// seq → seq symbol
void ScriptParserData::ReduceSeq1(const std::vector<std::unique_ptr<Property>>& props) {}

// symbol → id
void ScriptParserData::ReduceSymbol0(const std::vector<std::unique_ptr<Property>>& props) {}

// symbol → terminal priorityT
void ScriptParserData::ReduceSymbol1(const std::vector<std::unique_ptr<Property>>& props) {}

// priorityT → ε
void ScriptParserData::ReducePriorityT0(const std::vector<std::unique_ptr<Property>>& props) {}

// priorityT → priorityMarkTerminal
void ScriptParserData::ReducePriorityT1(const std::vector<std::unique_ptr<Property>>& props) {}

// tokens → token
void ScriptParserData::ReduceTokens0(const std::vector<std::unique_ptr<Property>>& props) {}

// tokens → tokens token
void ScriptParserData::ReduceTokens1(const std::vector<std::unique_ptr<Property>>& props) {}

// token → id : string ;
void ScriptParserData::ReduceToken(const std::vector<std::unique_ptr<Property>>& props) {}

}  // namespace cc