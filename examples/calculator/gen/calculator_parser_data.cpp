//
// Generated Parser file by stupidcoder cc
// Date: 2026-07-06 05:19
//

#include "calculator_parser_data.h"

namespace cal::gen {

// ==================== Token Structs Impl ====================

int TokenNumber::Type() {
    return kTokenNumber;
}
// ==================== Lexer Data ====================

int CalculatorLexerData::CharClassCount() {
    return 4;
}

int CalculatorLexerData::StatesCount() {
    return 5;
}

int CalculatorLexerData::StartState() {
    return 0;
}

void CalculatorLexerData::InitAccepted(std::vector<bool>& vec) {
    vec[3] = true;
    vec[2] = true;
    vec[1] = true;
}

void CalculatorLexerData::InitGoto(std::vector<std::vector<int>>& vec) {
    // 三元组：{当前状态, 输入类, 目标状态}
    struct Trans {
        int s, c, t;
    };
    std::vector<Trans> trans = {
            { 0, 1, 2 },
            { 0, 3, 1 },
            { 1, 2, 4 },
            { 1, 3, 1 },
            { 3, 3, 3 },
            { 4, 3, 3 },
    };
    for (const auto& t : trans) {
        vec[t.s][t.c] = t.t;
    }
}

void CalculatorLexerData::InitCharToClass(std::vector<int>& vec) {
    vec['('] = 1;
    vec[')'] = 1;
    vec['*'] = 1;
    vec['+'] = 1;
    vec['-'] = 1;
    vec['.'] = 2;
    vec['/'] = 1;
    vec['0'] = 3;
    vec['1'] = 3;
    vec['2'] = 3;
    vec['3'] = 3;
    vec['4'] = 3;
    vec['5'] = 3;
    vec['6'] = 3;
    vec['7'] = 3;
    vec['8'] = 3;
    vec['^'] = 1;
}

void CalculatorLexerData::InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) {
    vec[3] = [] { return std::make_unique<TokenNumber>(); };
    vec[2] = [] { return std::make_unique<TokenSingle>(); };
    vec[1] = [] { return std::make_unique<TokenNumber>(); };
}

// ==================== Parser Data ====================

int CalculatorParserData::TokenMappersCount() const {
    return 128;
}
int CalculatorParserData::NonTerminalSymbolsCount() const {
    return 2;
}
int CalculatorParserData::TerminalSymbolsCount() const {
    return 9;
}
int CalculatorParserData::StatesCount() const {
    return 18;
}
int CalculatorParserData::ProductionsCount() const {
    return 9;
}

void CalculatorParserData::InitReduceActions(std::vector<common::ReduceFunc>& vec) {
    vec[0] = [this](auto& p) { ReduceRoot(p); };  // ROOT → expr
    vec[1] = [this](auto& p) { ReduceExpr0(p); };  // expr → expr '+' expr
    vec[2] = [this](auto& p) { ReduceExpr1(p); };  // expr → expr '-' expr
    vec[3] = [this](auto& p) { ReduceExpr2(p); };  // expr → expr '*' expr
    vec[4] = [this](auto& p) { ReduceExpr3(p); };  // expr → expr '/' expr
    vec[5] = [this](auto& p) { ReduceExpr4(p); };  // expr → expr '^' expr
    vec[6] = [this](auto& p) { ReduceExpr5(p); };  // expr → '-' expr
    vec[7] = [this](auto& p) { ReduceExpr6(p); };  // expr → '(' expr ')'
    vec[8] = [this](auto& p) { ReduceExpr7(p); };  // expr → @number
}

void CalculatorParserData::InitGoto(std::vector<std::vector<int>>& vec) {
    vec[0][0] = 4;
    vec[1][0] = 5;
    vec[3][0] = 6;
    vec[7][0] = 13;
    vec[8][0] = 14;
    vec[9][0] = 15;
    vec[10][0] = 16;
    vec[11][0] = 17;
}

void CalculatorParserData::InitActions(std::vector<std::vector<int>>& vec) {
    using common::kActionAccept;
    using common::kActionReduce;
    using common::kActionShift;
    vec[0][4] = kActionShift | 1;
    vec[0][6] = kActionShift | 2;
    vec[0][7] = kActionShift | 3;
    vec[1][4] = kActionShift | 1;
    vec[1][6] = kActionShift | 2;
    vec[1][7] = kActionShift | 3;
    vec[2][0] = kActionReduce | 8;
    vec[2][1] = kActionReduce | 8;
    vec[2][2] = kActionReduce | 8;
    vec[2][3] = kActionReduce | 8;
    vec[2][5] = kActionReduce | 8;
    vec[2][7] = kActionReduce | 8;
    vec[2][8] = kActionReduce | 8;
    vec[3][4] = kActionShift | 1;
    vec[3][6] = kActionShift | 2;
    vec[3][7] = kActionShift | 3;
    vec[4][0] = kActionAccept;
    vec[4][2] = kActionShift | 7;
    vec[4][3] = kActionShift | 8;
    vec[4][5] = kActionShift | 9;
    vec[4][7] = kActionShift | 10;
    vec[4][8] = kActionShift | 11;
    vec[5][1] = kActionShift | 12;
    vec[5][2] = kActionShift | 7;
    vec[5][3] = kActionShift | 8;
    vec[5][5] = kActionShift | 9;
    vec[5][7] = kActionShift | 10;
    vec[5][8] = kActionShift | 11;
    vec[6][0] = kActionReduce | 6;
    vec[6][1] = kActionReduce | 6;
    vec[6][2] = kActionReduce | 6;
    vec[6][3] = kActionReduce | 6;
    vec[6][5] = kActionReduce | 6;
    vec[6][7] = kActionReduce | 6;
    vec[6][8] = kActionReduce | 6;
    vec[7][4] = kActionShift | 1;
    vec[7][6] = kActionShift | 2;
    vec[7][7] = kActionShift | 3;
    vec[8][4] = kActionShift | 1;
    vec[8][6] = kActionShift | 2;
    vec[8][7] = kActionShift | 3;
    vec[9][4] = kActionShift | 1;
    vec[9][6] = kActionShift | 2;
    vec[9][7] = kActionShift | 3;
    vec[10][4] = kActionShift | 1;
    vec[10][6] = kActionShift | 2;
    vec[10][7] = kActionShift | 3;
    vec[11][4] = kActionShift | 1;
    vec[11][6] = kActionShift | 2;
    vec[11][7] = kActionShift | 3;
    vec[12][0] = kActionReduce | 7;
    vec[12][1] = kActionReduce | 7;
    vec[12][2] = kActionReduce | 7;
    vec[12][3] = kActionReduce | 7;
    vec[12][5] = kActionReduce | 7;
    vec[12][7] = kActionReduce | 7;
    vec[12][8] = kActionReduce | 7;
    vec[13][0] = kActionReduce | 5;
    vec[13][1] = kActionReduce | 5;
    vec[13][2] = kActionReduce | 5;
    vec[13][3] = kActionReduce | 5;
    vec[13][5] = kActionReduce | 5;
    vec[13][7] = kActionReduce | 5;
    vec[13][8] = kActionReduce | 5;
    vec[14][0] = kActionReduce | 4;
    vec[14][1] = kActionReduce | 4;
    vec[14][2] = kActionShift | 7;
    vec[14][3] = kActionReduce | 4;
    vec[14][5] = kActionReduce | 4;
    vec[14][7] = kActionReduce | 4;
    vec[14][8] = kActionReduce | 4;
    vec[15][0] = kActionReduce | 3;
    vec[15][1] = kActionReduce | 3;
    vec[15][2] = kActionShift | 7;
    vec[15][3] = kActionReduce | 3;
    vec[15][5] = kActionReduce | 3;
    vec[15][7] = kActionReduce | 3;
    vec[15][8] = kActionReduce | 3;
    vec[16][0] = kActionReduce | 2;
    vec[16][1] = kActionReduce | 2;
    vec[16][2] = kActionShift | 7;
    vec[16][3] = kActionShift | 8;
    vec[16][5] = kActionShift | 9;
    vec[16][7] = kActionReduce | 2;
    vec[16][8] = kActionReduce | 2;
    vec[17][0] = kActionReduce | 1;
    vec[17][1] = kActionReduce | 1;
    vec[17][2] = kActionShift | 7;
    vec[17][3] = kActionShift | 8;
    vec[17][5] = kActionShift | 9;
    vec[17][7] = kActionReduce | 1;
    vec[17][8] = kActionReduce | 1;
}

void CalculatorParserData::InitTokenToSymbol(std::vector<int>& vec) {
    vec[')'] = 1;
    vec['^'] = 2;
    vec['/'] = 3;
    vec['('] = 4;
    vec['*'] = 5;
    vec[kTokenNumber] = 6;
    vec['-'] = 7;
    vec['+'] = 8;
}

void CalculatorParserData::InitProductions(std::vector<common::Production>& prods) {
    // Productions
    prods[0] = { 0, { false, 1 }, {{ false, 0 }, }}; // ROOT → expr
    prods[1] = { 1, { false, 0 }, {{ false, 0 }, { true, 8 }, { false, 0 }, }}; // expr → expr '+' expr
    prods[2] = { 2, { false, 0 }, {{ false, 0 }, { true, 7 }, { false, 0 }, }}; // expr → expr '-' expr
    prods[3] = { 3, { false, 0 }, {{ false, 0 }, { true, 5 }, { false, 0 }, }}; // expr → expr '*' expr
    prods[4] = { 4, { false, 0 }, {{ false, 0 }, { true, 3 }, { false, 0 }, }}; // expr → expr '/' expr
    prods[5] = { 5, { false, 0 }, {{ false, 0 }, { true, 2 }, { false, 0 }, }}; // expr → expr '^' expr
    prods[6] = { 6, { false, 0 }, {{ true, 7 }, { false, 0 }, }}; // expr → '-' expr
    prods[7] = { 7, { false, 0 }, {{ true, 4 }, { false, 0 }, { true, 1 }, }}; // expr → '(' expr ')'
    prods[8] = { 8, { false, 0 }, {{ true, 6 }, }}; // expr → @number
}

// ROOT → expr
void CalculatorParserData::ReduceRoot(const std::vector<std::unique_ptr<Property>>& props){}
// expr → expr '+' expr
void CalculatorParserData::ReduceExpr0(const std::vector<std::unique_ptr<Property>>& props){}
// expr → expr '-' expr
void CalculatorParserData::ReduceExpr1(const std::vector<std::unique_ptr<Property>>& props){}
// expr → expr '*' expr
void CalculatorParserData::ReduceExpr2(const std::vector<std::unique_ptr<Property>>& props){}
// expr → expr '/' expr
void CalculatorParserData::ReduceExpr3(const std::vector<std::unique_ptr<Property>>& props){}
// expr → expr '^' expr
void CalculatorParserData::ReduceExpr4(const std::vector<std::unique_ptr<Property>>& props){}
// expr → '-' expr
void CalculatorParserData::ReduceExpr5(const std::vector<std::unique_ptr<Property>>& props){}
// expr → '(' expr ')'
void CalculatorParserData::ReduceExpr6(const std::vector<std::unique_ptr<Property>>& props){}
// expr → @number
void CalculatorParserData::ReduceExpr7(const std::vector<std::unique_ptr<Property>>& props){}

}  // namespace cal::gen
