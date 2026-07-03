//
// Created by PC on 2026/7/2.
//

#include "script_parser_data.h"

#include <ranges>

namespace cc::generate {

// ==================== Token Structs Impl ====================

int TokenId::Type() {
    return kTokenId;
}
int TokenStringVal::Type() {
    return kTokenString;
}
int TokenTerminal::Type() {
    return kTokenTerminal;
}
int TokenProdMark::Type() {
    return kTokenProdMark;
}
int TokenSymbMark::Type() {
    return kTokenSymbMark;
}
int TokenTokenBegin::Type() {
    return kTokenTokenBegin;
}
int TokenSyntaxBegin::Type() {
    return kTokenSyntaxBegin;
}
int TokenBlockEnd::Type() {
    return kTokenBlockEnd;
}

// ==================== Lexer Data ====================

int ScriptLexerData::CharClassCount() {
    return 22;
}
int ScriptLexerData::StatesCount() {
    return 19;
}
int ScriptLexerData::StartState() {
    return 16;
}

void ScriptLexerData::InitAccepted(std::vector<bool>& vec) {
    vec.assign(19, false);
    vec[1] = true;   // string
    vec[2] = true;   // syntax_begin
    vec[3] = true;   // token_begin
    vec[4] = true;   // prod_mark
    vec[5] = true;   // block_end
    vec[6] = true;   // symb_mark
    vec[7] = true;   // terminal
    vec[8] = true;   // single
    vec[9] = true;   // id
    vec[17] = true;  // terminal
}

void ScriptLexerData::InitGoto(std::vector<std::vector<int>>& vec) {
    vec.assign(19, std::vector<int>(22, -1));

    // State 0
    vec[0][0] = 0;
    vec[0][1] = 1;
    for (int c = 2; c < 22; ++c)
        vec[0][c] = 0;

    // State 1: no outgoing (accept)
    // State 2: no outgoing (accept)
    // State 3: no outgoing (accept)
    // State 4: no outgoing (accept)

    // State 5
    vec[5][15] = 18;
    vec[5][16] = 18;

    // State 6: no outgoing (accept)

    // State 7
    vec[7][4] = 17;

    // State 8
    for (int c : {5, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20})
        vec[8][c] = 8;

    // State 9: no outgoing (accept)

    // State 10
    vec[10][17] = 2;

    // State 11
    for (int c = 0; c < 19; ++c)
        vec[11][c] = 18;
    for (int c = 20; c < 22; ++c)
        vec[11][c] = 18;
    vec[11][19] = 11;

    // State 12
    vec[12][13] = 3;

    // State 13
    vec[13][3] = 5;
    vec[13][5] = 15;
    vec[13][12] = 4;

    // State 14
    vec[14][5] = 14;
    vec[14][12] = 6;

    // State 15
    vec[15][5] = 15;
    vec[15][12] = 4;

    // State 16 (start)
    vec[16][1] = 0;
    vec[16][2] = 14;
    vec[16][3] = 13;
    vec[16][4] = 11;
    vec[16][6] = 9;
    vec[16][7] = 18;
    vec[16][8] = 8;
    for (int c = 9; c < 19; ++c)
        vec[16][c] = 8;
    // 类0,5,19,20,21 无转移，保持 -1

    // State 17
    for (int c : {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18})
        vec[17][c] = 17;

    // State 18
    vec[18][2] = 18;
    for (int c : {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18})
        vec[18][c] = 17;
    vec[18][21] = 17;
}

void ScriptLexerData::InitCharToClass(std::vector<int>& vec) {
    vec.assign(128, 0);  // 所有字符默认类0

    // 类1：双引号 "
    vec['"'] = 1;

    // 类2：美元符 $
    vec['$'] = 2;

    // 类3：百分号 %
    vec['%'] = 3;

    // 类4：单引号 '
    vec['\''] = 4;

    // 类5：数字 0~8
    for (char c = '0'; c <= '8'; ++c)
        vec[c] = 5;

    // 类6：冒号、分号、竖线
    vec[':'] = 6;
    vec[';'] = 6;
    vec['|'] = 6;

    // 类7：@
    vec['@'] = 7;

    // 类8：A
    vec['A'] = 8;

    // 类9：B C D F G H I J M P Q U V W 及小写 (不含 l, r, z)
    for (char c : "BCDFGHIJMPQUVW")
        vec[c] = 9;
    for (char c : "abcdefghijkmnopqstuvwxy")
        vec[c] = 9;  // 注意 l,r,z 不在内

    // 类10：E
    vec['E'] = 10;

    // 类11：K
    vec['K'] = 11;

    // 类12：L R l r
    vec['L'] = 12;
    vec['R'] = 12;
    vec['l'] = 12;
    vec['r'] = 12;

    // 类13：N
    vec['N'] = 13;

    // 类14：O
    vec['O'] = 14;

    // 类15：S
    vec['S'] = 15;

    // 类16：T
    vec['T'] = 16;

    // 类17：X
    vec['X'] = 17;

    // 类18：Y
    vec['Y'] = 18;

    // 类19：反斜杠
    vec['\\'] = 19;

    // 类20：下划线
    vec['_'] = 20;

    // 类21：波浪号
    vec['~'] = 21;

    // 其余字符（空格、数字9、大写Z、小写z、各种标点）默认为0，无需修改
}

void ScriptLexerData::InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) {
    vec.resize(19);
    vec[1] = [] { return std::make_unique<TokenStringVal>(); };
    vec[2] = [] { return std::make_unique<TokenSyntaxBegin>(); };
    vec[3] = [] { return std::make_unique<TokenTokenBegin>(); };
    vec[4] = [] { return std::make_unique<TokenProdMark>(); };
    vec[5] = [] { return std::make_unique<TokenBlockEnd>(); };
    vec[6] = [] { return std::make_unique<TokenSymbMark>(); };
    vec[7] = [] { return std::make_unique<TokenTerminal>(); };
    vec[8] = [] { return std::make_unique<common::TokenSingle>(); };
    vec[9] = [] { return std::make_unique<TokenId>(); };
    vec[17] = [] { return std::make_unique<TokenTerminal>(); };
}

// ==================== Parser Data ====================

// Terminal symbols: 12 (11 terminals + $)
//   Parser ID 0:  $ (EOF)
//   Parser ID 1:  symb_mark      (LALR term 0)
//   Parser ID 2:  terminal        (LALR term 1)
//   Parser ID 3:  prod_mark       (LALR term 2)
//   Parser ID 4:  |               (LALR term 3)
//   Parser ID 5:  ;               (LALR term 4)
//   Parser ID 6:  string          (LALR term 5)
//   Parser ID 7:  :               (LALR term 6)
//   Parser ID 8:  id              (LALR term 7)
//   Parser ID 9:  syntax_begin    (LALR term 8)
//   Parser ID 10: block_end       (LALR term 9)
//   Parser ID 11: token_begin     (LALR term 10)

// Non-terminal symbols (12):
//   Parser ID 0:  seq             (LALR nonterm 11)
//   Parser ID 1:  slice           (LALR nonterm 12)
//   Parser ID 2:  body            (LALR nonterm 13)
//   Parser ID 3:  prod_priority   (LALR nonterm 14)
//   Parser ID 4:  token           (LALR nonterm 15)
//   Parser ID 5:  symbol          (LALR nonterm 16)
//   Parser ID 6:  prod            (LALR nonterm 17)
//   Parser ID 7:  syntax          (LALR nonterm 18)
//   Parser ID 8:  tokens          (LALR nonterm 19)
//   Parser ID 9:  symb_priority   (LALR nonterm 20)
//   Parser ID 10: script          (LALR nonterm 21)
//   Parser ID 11: ROOT

int ScriptParserData::TokenMappersCount() const {
    return 264;
}  // 0..263
int ScriptParserData::NonTerminalSymbolsCount() const {
    return 12;
}
int ScriptParserData::TerminalSymbolsCount() const {
    return 12;
}  // 11 terminals + $
int ScriptParserData::StatesCount() const {
    return 32;
}

void ScriptParserData::InitReduceActions(std::vector<common::ReduceFunc>& vec) {
    vec.resize(19);
    // prod 0: ROOT -> script
    vec[0] = [this](auto& p) { ReduceRoot(p); };
    // prod 1: script -> token_begin tokens block_end syntax_begin syntax block_end
    vec[1] = [this](auto& p) { ReduceScript(p); };
    // prod 2: tokens -> token
    vec[2] = [this](auto& p) { ReduceTokens0(p); };
    // prod 3: tokens -> tokens token
    vec[3] = [this](auto& p) { ReduceTokens1(p); };
    // prod 4: token -> id : string ;
    vec[4] = [this](auto& p) { ReduceToken(p); };
    // prod 5: syntax -> prod
    vec[5] = [this](auto& p) { ReduceSyntax0(p); };
    // prod 6: syntax -> syntax prod
    vec[6] = [this](auto& p) { ReduceSyntax1(p); };
    // prod 7: prod -> id : body ;
    vec[7] = [this](auto& p) { ReduceProd(p); };
    // prod 8: body -> slice
    vec[8] = [this](auto& p) { ReduceBody0(p); };
    // prod 9: body -> body | slice
    vec[9] = [this](auto& p) { ReduceBody1(p); };
    // prod 10: slice -> seq prod_priority
    vec[10] = [this](auto& p) { ReduceSlice(p); };
    // prod 11: prod_priority -> ε
    vec[11] = [this](auto& p) { ReduceProdPriority0(p); };
    // prod 12: prod_priority -> prod_mark
    vec[12] = [this](auto& p) { ReduceProdPriority1(p); };
    // prod 13: seq -> symbol
    vec[13] = [this](auto& p) { ReduceSeq0(p); };
    // prod 14: seq -> seq symbol
    vec[14] = [this](auto& p) { ReduceSeq1(p); };
    // prod 15: symbol -> id
    vec[15] = [this](auto& p) { ReduceSymbol0(p); };
    // prod 16: symbol -> terminal symb_priority
    vec[16] = [this](auto& p) { ReduceSymbol1(p); };
    // prod 17: symb_priority -> ε
    vec[17] = [this](auto& p) { ReduceSymbPriority0(p); };
    // prod 18: symb_priority -> symb_mark
    vec[18] = [this](auto& p) { ReduceSymbPriority1(p); };
}

void ScriptParserData::InitGoto(std::vector<std::vector<int>>& vec) {
    vec.assign(32, std::vector(12, 0));

    // GOTO: parser nonterm ID = LALR nonterm - 11
    vec[0][10] = 2;   // script (LALR 21→2)
    vec[1][4] = 4;    // token (LALR 15→4)
    vec[1][8] = 5;    // tokens (LALR 19→5)
    vec[5][4] = 8;    // token (LALR 15→8)
    vec[10][6] = 13;  // prod (LALR 17→13)
    vec[10][7] = 14;  // syntax (LALR 18→14)
    vec[14][6] = 17;  // prod (LALR 17→17)
    vec[15][0] = 20;  // seq (LALR 11→20)
    vec[15][1] = 21;  // slice (LALR 12→21)
    vec[15][2] = 22;  // body (LALR 13→22)
    vec[15][5] = 23;  // symbol (LALR 16→23)
    vec[18][9] = 25;  // symb_priority (LALR 20→25)
    vec[20][3] = 27;  // prod_priority (LALR 14→27)
    vec[20][5] = 28;  // symbol (LALR 16→28)
    vec[29][0] = 20;  // seq (LALR 11→20)
    vec[29][1] = 31;  // slice (LALR 12→31)
    vec[29][5] = 23;  // symbol (LALR 16→23)
}

void ScriptParserData::InitActions(std::vector<std::vector<int>>& vec) {
    vec.assign(32, std::vector(12, 0));

    // LALR dump type: 1=kShift→common::kActionShift, 2=kReduce→common::kActionReduce, 3=kAccept→common::kActionAccept
    // LALR term → Parser term: 0→1, 1→2, ..., 10→11, 23→0

    // State 0
    vec[0][11] = common::kActionShift | 1;  // LALR: 0/10 ACCEPT 1 → SHIFT token_begin→1

    // State 1
    vec[1][8] = common::kActionShift | 3;  // LALR: 1/7 ACCEPT 3 → SHIFT id→3

    // State 2
    vec[2][0] = common::kActionAccept;  // LALR: 2/23 REDUCE -1 → ACCEPT

    // State 3
    vec[3][7] = common::kActionShift | 9;  // LALR: 3/6 ACCEPT 9 → SHIFT :→9

    // State 4
    vec[4][8] = common::kActionReduce | 2;   // LALR: 4/7 SHIFT 2 → REDUCE prod 2 (tokens→token)
    vec[4][10] = common::kActionReduce | 2;  // LALR: 4/9 SHIFT 2 → REDUCE prod 2

    // State 5
    vec[5][8] = common::kActionShift | 3;   // LALR: 5/7 ACCEPT 3 → SHIFT id→3
    vec[5][10] = common::kActionShift | 7;  // LALR: 5/9 ACCEPT 7 → SHIFT block_end→7

    // State 6
    vec[6][6] = common::kActionShift | 9;  // LALR: 6/5 ACCEPT 9 → SHIFT string→9

    // State 7
    vec[7][9] = common::kActionShift | 10;  // LALR: 7/8 ACCEPT 10 → SHIFT syntax_begin→10

    // State 8
    vec[8][8] =
            common::kActionReduce | 3;  // LALR: 8/7 SHIFT 3 → REDUCE prod 3 (tokens→tokens token)
    vec[8][10] = common::kActionReduce | 3;  // LALR: 8/9 SHIFT 3

    // State 9
    vec[9][5] = common::kActionShift | 11;  // LALR: 9/4 ACCEPT 11 → SHIFT ;→11

    // State 10
    vec[10][8] = common::kActionShift | 12;  // LALR: 10/7 ACCEPT 12 → SHIFT id→12

    // State 11
    vec[11][8] =
            common::kActionReduce | 4;  // LALR: 11/7 SHIFT 4 → REDUCE prod 4 (token→id : string ;)
    vec[11][10] = common::kActionReduce | 4;  // LALR: 11/9 SHIFT 4

    // State 12
    vec[12][7] = common::kActionShift | 15;  // LALR: 12/6 ACCEPT 15 → SHIFT :→15

    // State 13
    vec[13][8] = common::kActionReduce | 5;   // LALR: 13/7 SHIFT 5 → REDUCE prod 5 (syntax→prod)
    vec[13][10] = common::kActionReduce | 5;  // LALR: 13/9 SHIFT 5

    // State 14
    vec[14][8] = common::kActionShift | 12;   // LALR: 14/7 ACCEPT 12 → SHIFT id→12
    vec[14][10] = common::kActionShift | 16;  // LALR: 14/9 ACCEPT 16 → SHIFT prod_mark→16

    // State 15
    vec[15][2] = common::kActionShift | 18;  // LALR: 15/1 ACCEPT 18 → SHIFT terminal→18
    vec[15][8] = common::kActionShift | 19;  // LALR: 15/7 ACCEPT 19 → SHIFT id→19

    // State 16
    vec[16][0] = common::kActionReduce | 1;  // LALR: 16/23 SHIFT 1 → REDUCE prod 1 (script→...)

    // State 17
    vec[17][8] =
            common::kActionReduce | 6;  // LALR: 17/7 SHIFT 6 → REDUCE prod 6 (syntax→syntax prod)
    vec[17][10] = common::kActionReduce | 6;  // LALR: 17/9 SHIFT 6

    // State 18
    vec[18][0] = common::kActionAccept;  // LALR: 18/0 REDUCE 24 → ACCEPT
    vec[18][2] =
            common::kActionReduce | 17;  // LALR: 18/1 SHIFT 17 → REDUCE prod 17 (symb_priority→ε)
    vec[18][3] = common::kActionReduce | 17;  // LALR: 18/2 SHIFT 17
    vec[18][4] = common::kActionReduce | 17;  // LALR: 18/3 SHIFT 17
    vec[18][5] = common::kActionReduce | 17;  // LALR: 18/4 SHIFT 17
    vec[18][8] = common::kActionReduce | 17;  // LALR: 18/7 SHIFT 17

    // State 19
    vec[19][2] = common::kActionReduce | 15;  // LALR: 19/1 SHIFT 15 → REDUCE prod 15 (symbol→id)
    vec[19][3] = common::kActionReduce | 15;
    vec[19][4] = common::kActionReduce | 15;
    vec[19][5] = common::kActionReduce | 15;
    vec[19][8] = common::kActionReduce | 15;

    // State 20
    vec[20][2] = common::kActionShift | 18;  // LALR: 20/1 ACCEPT 18 → SHIFT terminal→18
    vec[20][3] = common::kActionShift | 26;  // LALR: 20/2 ACCEPT 26 → SHIFT prod_mark→26
    vec[20][4] =
            common::kActionReduce | 11;  // LALR: 20/3 SHIFT 11 → REDUCE prod 11 (prod_priority→ε)
    vec[20][5] = common::kActionReduce | 11;  // LALR: 20/4 SHIFT 11
    vec[20][8] = common::kActionShift | 19;   // LALR: 20/7 ACCEPT 19 → SHIFT id→19

    // State 21
    vec[21][4] = common::kActionReduce | 8;  // LALR: 21/3 SHIFT 8 → REDUCE prod 8 (body→slice)
    vec[21][5] = common::kActionReduce | 8;  // LALR: 21/4 SHIFT 8

    // State 22
    vec[22][4] = common::kActionShift | 29;  // LALR: 22/3 ACCEPT 29 → SHIFT |→29
    vec[22][5] = common::kActionShift | 30;  // LALR: 22/4 ACCEPT 30 → SHIFT ;→30

    // State 23
    vec[23][2] = common::kActionReduce | 13;  // LALR: 23/1 SHIFT 13 → REDUCE prod 13 (seq→symbol)
    vec[23][3] = common::kActionReduce | 13;
    vec[23][4] = common::kActionReduce | 13;
    vec[23][5] = common::kActionReduce | 13;
    vec[23][8] = common::kActionReduce | 13;

    // State 24
    vec[24][2] = common::kActionReduce |
                 18;  // LALR: 24/1 SHIFT 18 → REDUCE prod 18 (symb_priority→symb_mark)
    vec[24][3] = common::kActionReduce | 18;
    vec[24][4] = common::kActionReduce | 18;
    vec[24][5] = common::kActionReduce | 18;
    vec[24][8] = common::kActionReduce | 18;

    // State 25
    vec[25][2] = common::kActionReduce |
                 16;  // LALR: 25/1 SHIFT 16 → REDUCE prod 16 (symbol→terminal symb_priority)
    vec[25][3] = common::kActionReduce | 16;
    vec[25][4] = common::kActionReduce | 16;
    vec[25][5] = common::kActionReduce | 16;
    vec[25][8] = common::kActionReduce | 16;

    // State 26
    vec[26][4] = common::kActionReduce |
                 12;  // LALR: 26/3 SHIFT 12 → REDUCE prod 12 (prod_priority→prod_mark)
    vec[26][5] = common::kActionReduce | 12;  // LALR: 26/4 SHIFT 12

    // State 27
    vec[27][4] = common::kActionReduce |
                 10;  // LALR: 27/3 SHIFT 10 → REDUCE prod 10 (slice→seq prod_priority)
    vec[27][5] = common::kActionReduce | 10;  // LALR: 27/4 SHIFT 10

    // State 28
    vec[28][2] =
            common::kActionReduce | 14;  // LALR: 28/1 SHIFT 14 → REDUCE prod 14 (seq→seq symbol)
    vec[28][3] = common::kActionReduce | 14;
    vec[28][4] = common::kActionReduce | 14;
    vec[28][5] = common::kActionReduce | 14;
    vec[28][8] = common::kActionReduce | 14;

    // State 29
    vec[29][2] = common::kActionShift | 18;  // LALR: 29/1 ACCEPT 18 → SHIFT terminal→18
    vec[29][8] = common::kActionShift | 19;  // LALR: 29/7 ACCEPT 19 → SHIFT id→19

    // State 30
    vec[30][8] =
            common::kActionReduce | 7;  // LALR: 30/7 SHIFT 7 → REDUCE prod 7 (prod→id : body ;)
    vec[30][10] = common::kActionReduce | 7;  // LALR: 30/9 SHIFT 7

    // State 31
    vec[31][4] =
            common::kActionReduce | 9;  // LALR: 31/3 SHIFT 9 → REDUCE prod 9 (body→body | slice)
    vec[31][5] = common::kActionReduce | 9;  // LALR: 31/4 SHIFT 9
}

void ScriptParserData::InitTokenToSymbol(std::vector<int>& vec) {
    vec.assign(264, 0);
    // Map lexer token types → Parser terminal IDs
    vec[0] = 0;                  // TokenEof → $
    vec[':'] = 7;                // : → term 7
    vec[';'] = 5;                // ; → term 5
    vec['|'] = 4;                // | → term 4
    vec[kTokenId] = 8;           // id → term 8
    vec[kTokenString] = 6;       // string → term 6
    vec[kTokenTerminal] = 2;     // terminal → term 2
    vec[kTokenProdMark] = 3;     // prod_mark → term 3
    vec[kTokenSymbMark] = 1;     // symb_mark → term 1
    vec[kTokenTokenBegin] = 11;  // token_begin → term 11
    vec[kTokenSyntaxBegin] = 9;  // syntax_begin → term 9
    vec[kTokenBlockEnd] = 10;    // block_end → term 10
}

void ScriptParserData::InitPropertySuppliers(std::vector<common::PropertySupplier>& vec) {
    vec.resize(12);
    for (auto& s : vec)
        s = [] { return std::make_unique<PropertyVoid>(); };
}

void ScriptParserData::InitSymbolsAndProductions(
        std::vector<common::Symbol>& symbols, std::vector<common::Production>& prods) {
    // Symbols: non-terminals (12)
    symbols.resize(12);
    symbols[0] = {false, 0};    // seq
    symbols[1] = {false, 1};    // slice
    symbols[2] = {false, 2};    // body
    symbols[3] = {false, 3};    // prod_priority
    symbols[4] = {false, 4};    // token
    symbols[5] = {false, 5};    // symbol
    symbols[6] = {false, 6};    // prod
    symbols[7] = {false, 7};    // syntax
    symbols[8] = {false, 8};    // tokens
    symbols[9] = {false, 9};    // symb_priority
    symbols[10] = {false, 10};  // script
    symbols[11] = {false, 11};  // ROOT

    // Productions (19)
    prods.resize(19);
    prods[0] = {0, {false, 11}, {{false, 10}}};  // ROOT → script
    prods[1] = {1, {false, 10},
            {{true, 11}, {false, 8}, {true, 10}, {true, 9}, {false, 7}, {true, 10}}};  // script→...
    prods[2] = {2, {false, 8}, {{false, 4}}};              // tokens → token
    prods[3] = {3, {false, 8}, {{false, 8}, {false, 4}}};  // tokens → tokens token
    prods[4] = {
            4, {false, 4}, {{true, 8}, {true, 7}, {true, 6}, {true, 5}}};  // token → id : string ;
    prods[5] = {5, {false, 7}, {{false, 6}}};                              // syntax → prod
    prods[6] = {6, {false, 7}, {{false, 7}, {false, 6}}};                  // syntax → syntax prod
    prods[7] = {
            7, {false, 6}, {{true, 8}, {true, 7}, {false, 2}, {true, 5}}};  // prod → id : body ;
    prods[8] = {8, {false, 2}, {{false, 1}}};                               // body → slice
    prods[9] = {9, {false, 2}, {{false, 2}, {true, 4}, {false, 1}}};        // body → body | slice
    prods[10] = {10, {false, 1}, {{false, 0}, {false, 3}}};  // slice → seq prod_priority
    prods[11] = {11, {false, 3}, {}};                        // prod_priority → ε
    prods[12] = {12, {false, 3}, {{true, 3}}};               // prod_priority → prod_mark
    prods[13] = {13, {false, 0}, {{false, 5}}};              // seq → symbol
    prods[14] = {14, {false, 0}, {{false, 0}, {false, 5}}};  // seq → seq symbol
    prods[15] = {15, {false, 5}, {{true, 8}}};               // symbol → id
    prods[16] = {16, {false, 5}, {{true, 2}, {false, 9}}};   // symbol → terminal symb_priority
    prods[17] = {17, {false, 9}, {}};                        // symb_priority → ε
    prods[18] = {18, {false, 9}, {{true, 1}}};               // symb_priority → symb_mark
}

void ScriptParserData::ReduceRoot(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceScript(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceTokens0(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceTokens1(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceToken(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSyntax0(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSyntax1(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceProd(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceBody0(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceBody1(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSlice(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceProdPriority0(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceProdPriority1(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSeq0(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSeq1(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSymbol0(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSymbol1(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSymbPriority0(const std::vector<std::unique_ptr<Property>>& props) {}

void ScriptParserData::ReduceSymbPriority1(const std::vector<std::unique_ptr<Property>>& props) {}
}  // namespace cc::generate
