//
// Created by PC on 2026/7/2.
//

#include "script_parser_data.h"

namespace cc::gen {

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

TokenMatchResult TokenId::OnMatched(const std::string& lexeme, CompilerInput& ci) {
    val = lexeme;
    return TokenMatchResult::kAccept;
}

TokenMatchResult TokenStringVal::OnMatched(const std::string& lexeme, CompilerInput& ci) {
    val = lexeme.substr(1, lexeme.length() - 2);
    return TokenMatchResult::kAccept;
}

TokenMatchResult TokenTerminal::OnMatched(const std::string& lexeme, CompilerInput& ci) {
    if (lexeme[0] == '\'') {
        type = Type::kSingle;
        ch = lexeme[1];
        name = {ch};
        return TokenMatchResult::kAccept;
    }
    switch (lexeme[1]) {
        case '~':
            type = Type::kEpsilon;
            name = "ε";
            break;
        case '$':
            type = Type::kKeyWord;
            name = lexeme.substr(2);
            break;
        default:
            type = Type::kNormal;
            name = lexeme.substr(1);
            break;
    }
    return TokenMatchResult::kAccept;
}

TokenMatchResult TokenProdMark::OnMatched(const std::string& lexeme, CompilerInput& ci) {
    if (lexeme.back() == 'l' || lexeme.back() == 'L') {
        assoc = Associativity::kLeft;
    } else if (lexeme.back() == 'r' || lexeme.back() == 'R') {
        assoc = Associativity::kRight;
    } else {
        prior = std::stoi(lexeme.substr(1));
    }
    if (assoc != Associativity::kUndefined && lexeme.length() > 2) {
        prior = std::stoi(lexeme.substr(1, lexeme.length() - 2));
    }
    return TokenMatchResult::kAccept;
}

TokenMatchResult TokenSymbMark::OnMatched(const std::string& lexeme, CompilerInput& ci) {
    if (lexeme.back() == 'l' || lexeme.back() == 'L') {
        assoc = Associativity::kLeft;
    } else if (lexeme.back() == 'r' || lexeme.back() == 'R') {
        assoc = Associativity::kRight;
    } else {
        prior = std::stoi(lexeme.substr(1));
    }
    if (assoc != Associativity::kUndefined && lexeme.length() > 2) {
        prior = std::stoi(lexeme.substr(1, lexeme.length() - 2));
    }
    return TokenMatchResult::kAccept;
}

// ==================== Lexer Data ====================

int ScriptLexerData::CharClassCount() {
    return 22;
}

int ScriptLexerData::StatesCount() {
    return 31;
}

int ScriptLexerData::StartState() {
    return 13;
}

void ScriptLexerData::InitAccepted(std::vector<bool>& vec) {
    for (int i = 1; i <= 9; ++i) {
        vec[i] = true;
    }
    vec[14] = true;
    vec[16] = true;
    vec[17] = true;
    vec[18] = true;
    vec[22] = true;
}

void ScriptLexerData::InitGoto(std::vector<std::vector<int>>& vec) {
    // 三元组：{当前状态, 输入类, 目标状态}
    struct Trans {
        int s, c, t;
    };
    std::vector<Trans> trans = {// ========== 状态 0 ==========
            {0, 0, 25}, {0, 1, 1}, {0, 2, 25}, {0, 3, 25}, {0, 4, 25}, {0, 5, 25}, {0, 6, 25},
            {0, 7, 25}, {0, 8, 25}, {0, 9, 25}, {0, 10, 25}, {0, 11, 25}, {0, 12, 25}, {0, 13, 25},
            {0, 14, 25}, {0, 15, 25}, {0, 16, 25}, {0, 17, 25}, {0, 18, 25}, {0, 19, 0},
            {0, 20, 25}, {0, 21, 25},

            // ========== 状态 1 ==========
            {1, 0, 25}, {1, 1, 18}, {1, 2, 25}, {1, 3, 25}, {1, 4, 25}, {1, 5, 25}, {1, 6, 25},
            {1, 7, 25}, {1, 8, 25}, {1, 9, 25}, {1, 10, 25}, {1, 11, 25}, {1, 12, 25}, {1, 13, 25},
            {1, 14, 25}, {1, 15, 25}, {1, 16, 25}, {1, 17, 25}, {1, 18, 25}, {1, 19, 0},
            {1, 20, 25}, {1, 21, 25},

            // ========== 状态 2～3：无转移 ==========

            // ========== 状态 4 ==========
            {4, 15, 28}, {4, 16, 27},

            // ========== 状态 5 ==========
            {5, 5, 5}, {5, 8, 5}, {5, 9, 5}, {5, 10, 5}, {5, 11, 5}, {5, 12, 5}, {5, 13, 5},
            {5, 14, 5}, {5, 15, 5}, {5, 16, 5}, {5, 17, 5}, {5, 18, 5}, {5, 20, 5},

            // ========== 状态 6：无转移 ==========

            // ========== 状态 7 ==========
            {7, 3, 4}, {7, 5, 14}, {7, 12, 22},

            // ========== 状态 8 ==========
            {8, 4, 16},

            // ========== 状态 9 ==========
            {9, 5, 9}, {9, 12, 17},

            // ========== 状态 10 ==========
            {10, 17, 2},

            // ========== 状态 11 ==========
            {11, 0, 23}, {11, 1, 23}, {11, 2, 23}, {11, 3, 23}, {11, 4, 23}, {11, 5, 23},
            {11, 6, 23}, {11, 7, 23}, {11, 8, 23}, {11, 9, 23}, {11, 10, 23}, {11, 11, 23},
            {11, 12, 23}, {11, 13, 23}, {11, 14, 23}, {11, 15, 23}, {11, 16, 23}, {11, 17, 23},
            {11, 18, 23}, {11, 19, 20}, {11, 20, 23}, {11, 21, 23},

            // ========== 状态 12 ==========
            {12, 13, 3},

            // ========== 状态 13 ==========
            {13, 1, 25}, {13, 2, 9}, {13, 3, 7}, {13, 4, 11}, {13, 6, 6}, {13, 7, 29}, {13, 8, 5},
            {13, 9, 5}, {13, 10, 5}, {13, 11, 5}, {13, 12, 5}, {13, 13, 5}, {13, 14, 5},
            {13, 15, 5}, {13, 16, 5}, {13, 17, 5}, {13, 18, 5},

            // ========== 状态 14 ==========
            {14, 5, 14}, {14, 12, 22},

            // ========== 状态 15 ==========
            {15, 8, 10},

            // ========== 状态 16 ==========
            {16, 8, 16}, {16, 9, 16}, {16, 10, 16}, {16, 11, 16}, {16, 12, 16}, {16, 13, 16},
            {16, 14, 16}, {16, 15, 16}, {16, 16, 16}, {16, 17, 16}, {16, 18, 16},

            // ========== 状态 17～18：无转移 ==========

            // ========== 状态 19 ==========
            {19, 10, 12},

            // ========== 状态 20 ==========
            {20, 0, 23}, {20, 1, 23}, {20, 2, 23}, {20, 3, 23}, {20, 4, 8}, {20, 5, 23},
            {20, 6, 23}, {20, 7, 23}, {20, 8, 23}, {20, 9, 23}, {20, 10, 23}, {20, 11, 23},
            {20, 12, 23}, {20, 13, 23}, {20, 14, 23}, {20, 15, 23}, {20, 16, 23}, {20, 17, 23},
            {20, 18, 23}, {20, 19, 23}, {20, 20, 23}, {20, 21, 23},

            // ========== 状态 21 ==========
            {21, 16, 15},

            // ========== 状态 22：无转移 ==========

            // ========== 状态 23 ==========
            {23, 4, 16},

            // ========== 状态 24 ==========
            {24, 11, 19},

            // ========== 状态 25 ==========
            {25, 0, 25}, {25, 1, 18}, {25, 2, 25}, {25, 3, 25}, {25, 4, 25}, {25, 5, 25},
            {25, 6, 25}, {25, 7, 25}, {25, 8, 25}, {25, 9, 25}, {25, 10, 25}, {25, 11, 25},
            {25, 12, 25}, {25, 13, 25}, {25, 14, 25}, {25, 15, 25}, {25, 16, 25}, {25, 17, 25},
            {25, 18, 25}, {25, 19, 0}, {25, 20, 25}, {25, 21, 25},

            // ========== 状态 26 ==========
            {26, 13, 21},

            // ========== 状态 27 ==========
            {27, 14, 24},

            // ========== 状态 28 ==========
            {28, 18, 26},

            // ========== 状态 29 ==========
            {29, 2, 30}, {29, 8, 16}, {29, 9, 16}, {29, 10, 16}, {29, 11, 16}, {29, 12, 16},
            {29, 13, 16}, {29, 14, 16}, {29, 15, 16}, {29, 16, 16}, {29, 17, 16}, {29, 18, 16},
            {29, 21, 16},

            // ========== 状态 30 ==========
            {30, 8, 16}, {30, 9, 16}, {30, 10, 16}, {30, 11, 16}, {30, 12, 16}, {30, 13, 16},
            {30, 14, 16}, {30, 15, 16}, {30, 16, 16}, {30, 17, 16}, {30, 18, 16}};

    for (const auto& t : trans) {
        vec[t.s][t.c] = t.t;
    }
}

void ScriptLexerData::InitCharToClass(std::vector<int>& vec) {
    vec['"'] = 1;
    vec['$'] = 2;
    vec['%'] = 3;
    vec['\''] = 4;
    for (char c = '0'; c <= '8'; ++c)
        vec[c] = 5;
    vec[':'] = 6;
    vec[';'] = 6;
    vec['|'] = 6;
    vec['@'] = 7;
    vec['A'] = 8;
    // 类9：B C D F G H I J M P Q U V W 和 a~y (除去 l, r, z)
    for (char c : "BCDFGHIJMPQUVW")
        vec[c] = 9;
    for (char c : "abcdefghijkmnopqstuvwxy")
        vec[c] = 9;  // 排除 l, r, z
    vec['E'] = 10;
    vec['K'] = 11;
    vec['L'] = 12;
    vec['R'] = 12;
    vec['l'] = 12;
    vec['r'] = 12;
    vec['N'] = 13;
    vec['O'] = 14;
    vec['S'] = 15;
    vec['T'] = 16;
    vec['X'] = 17;
    vec['Y'] = 18;
    vec['\\'] = 19;
    vec['_'] = 20;
    vec['~'] = 21;
}

void ScriptLexerData::InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) {
    vec[1] = [] { return std::make_unique<TokenStringVal>(); };
    vec[2] = [] { return std::make_unique<TokenSyntaxBegin>(); };
    vec[3] = [] { return std::make_unique<TokenTokenBegin>(); };
    vec[4] = [] { return std::make_unique<TokenBlockEnd>(); };
    vec[5] = [] { return std::make_unique<TokenId>(); };
    vec[6] = [] { return std::make_unique<common::TokenSingle>(); };
    vec[7] = [] { return std::make_unique<TokenProdMark>(); };
    vec[8] = [] { return std::make_unique<TokenTerminal>(); };
    vec[9] = [] { return std::make_unique<TokenSymbMark>(); };
    vec[14] = [] { return std::make_unique<TokenProdMark>(); };
    vec[16] = [] { return std::make_unique<TokenTerminal>(); };
    vec[17] = [] { return std::make_unique<TokenSymbMark>(); };
    vec[18] = [] { return std::make_unique<TokenStringVal>(); };
    vec[22] = [] { return std::make_unique<TokenProdMark>(); };
}

// ==================== Parser Data ====================

int ScriptParserData::TokenMappersCount() const {
    return 264;
}  // 0..263
int ScriptParserData::NonTerminalSymbolsCount() const {
    return 13;
}
int ScriptParserData::TerminalSymbolsCount() const {
    return 12;
}  // 11 terminals + $
int ScriptParserData::StatesCount() const {
    return 33;
}
int ScriptParserData::ProductionsCount() const {
    return 20;
}

void ScriptParserData::InitReduceActions(std::vector<common::ReduceFunc>& vec) {
    // ROOT -> script
    // vec[0] = [this](auto& p) { ReduceRoot(p); };
    // script -> token_begin tokens block_end syntax_begin syntax block_end
    vec[1] = [this](auto& p) { ReduceScript(); };
    // tokens -> token
    // vec[2] = [this](auto& p) { ReduceTokens0(p); };
    // tokens -> tokens token
    // vec[3] = [this](auto& p) { ReduceTokens1(p); };
    // token -> id : string ;
    vec[4] = [this](auto& p) { ReduceToken(p); };
    // syntax -> prod
    // vec[5] = [this](auto& p) { ReduceSyntax0(p); };
    // syntax -> syntax prod
    // vec[6] = [this](auto& p) { ReduceSyntax1(p); };
    // prod -> id : body ;
    vec[7] = [this](auto& p) { ReduceProd(p); };
    // body -> slice
    // vec[8] = [this](auto& p) { ReduceBody0(p); };
    // body -> body | slice
    // vec[9] = [this](auto& p) { ReduceBody1(p); };
    // slice -> seq prod_priority
    vec[10] = [this](auto& p) { ReduceSlice(); };
    // prod_priority -> ε
    // vec[11] = [this](auto& p) { ReduceProdPriority0(p); };
    // prod_priority -> prod_mark
    vec[12] = [this](auto& p) { ReduceProdPriority1(p); };
    // seq -> symbol
    // vec[13] = [this](auto& p) { ReduceSeq0(p); };
    // seq -> seq symbol
    // vec[14] = [this](auto& p) { ReduceSeq1(p); };
    // symbol -> atom symb_priority
    // vec[15] = [this](auto& p) { ReduceSymbol(p); };
    // atom -> terminal
    vec[16] = [this](auto& p) { ReduceAtom0(p); };
    // atom -> id
    vec[17] = [this](auto& p) { ReduceAtom1(p); };
    // symb_priority -> ε
    // vec[18] = [this](auto& p) { ReduceSymbPriority0(p); };
    // symb_priority -> symb_mark
    vec[19] = [this](auto& p) { ReduceSymbPriority1(p); };
}

void ScriptParserData::InitGoto(std::vector<std::vector<int>>& vec) {
    vec[0][11] = 2;
    vec[1][4] = 4;
    vec[1][9] = 5;
    vec[5][4] = 8;
    vec[10][7] = 13;
    vec[10][8] = 14;
    vec[14][7] = 17;
    vec[15][0] = 20;
    vec[15][1] = 21;
    vec[15][2] = 22;
    vec[15][5] = 23;
    vec[15][6] = 24;
    vec[20][3] = 26;
    vec[20][5] = 23;
    vec[20][6] = 27;
    vec[23][10] = 31;
    vec[28][0] = 20;
    vec[28][1] = 32;
    vec[28][5] = 23;
    vec[28][6] = 24;
}

void ScriptParserData::InitActions(std::vector<std::vector<int>>& vec) {
    using common::kActionAccept;
    using common::kActionReduce;
    using common::kActionShift;
    vec[0][11] = kActionShift | 1;
    vec[1][8] = kActionShift | 3;
    vec[2][0] = kActionAccept;
    vec[3][7] = kActionShift | 6;
    vec[4][8] = kActionReduce | 2;
    vec[4][10] = kActionReduce | 2;
    vec[5][8] = kActionShift | 3;
    vec[5][10] = kActionShift | 7;
    vec[6][6] = kActionShift | 9;
    vec[7][9] = kActionShift | 10;
    vec[8][8] = kActionReduce | 3;
    vec[8][10] = kActionReduce | 3;
    vec[9][5] = kActionShift | 11;
    vec[10][8] = kActionShift | 12;
    vec[11][8] = kActionReduce | 4;
    vec[11][10] = kActionReduce | 4;
    vec[12][7] = kActionShift | 15;
    vec[13][8] = kActionReduce | 5;
    vec[13][10] = kActionReduce | 5;
    vec[14][8] = kActionShift | 12;
    vec[14][10] = kActionShift | 16;
    vec[15][2] = kActionShift | 18;
    vec[15][8] = kActionShift | 19;
    vec[16][0] = kActionReduce | 1;
    vec[17][8] = kActionReduce | 6;
    vec[17][10] = kActionReduce | 6;
    vec[18][1] = kActionReduce | 16;
    vec[18][2] = kActionReduce | 16;
    vec[18][3] = kActionReduce | 16;
    vec[18][4] = kActionReduce | 16;
    vec[18][5] = kActionReduce | 16;
    vec[18][8] = kActionReduce | 16;
    vec[19][1] = kActionReduce | 17;
    vec[19][2] = kActionReduce | 17;
    vec[19][3] = kActionReduce | 17;
    vec[19][4] = kActionReduce | 17;
    vec[19][5] = kActionReduce | 17;
    vec[19][8] = kActionReduce | 17;
    vec[20][2] = kActionShift | 18;
    vec[20][3] = kActionShift | 25;
    vec[20][4] = kActionReduce | 11;
    vec[20][5] = kActionReduce | 11;
    vec[20][8] = kActionShift | 19;
    vec[21][4] = kActionReduce | 8;
    vec[21][5] = kActionReduce | 8;
    vec[22][4] = kActionShift | 28;
    vec[22][5] = kActionShift | 29;
    vec[23][1] = kActionShift | 30;
    vec[23][2] = kActionReduce | 18;
    vec[23][3] = kActionReduce | 18;
    vec[23][4] = kActionReduce | 18;
    vec[23][5] = kActionReduce | 18;
    vec[23][8] = kActionReduce | 18;
    vec[24][2] = kActionReduce | 13;
    vec[24][3] = kActionReduce | 13;
    vec[24][4] = kActionReduce | 13;
    vec[24][5] = kActionReduce | 13;
    vec[24][8] = kActionReduce | 13;
    vec[25][4] = kActionReduce | 12;
    vec[25][5] = kActionReduce | 12;
    vec[26][4] = kActionReduce | 10;
    vec[26][5] = kActionReduce | 10;
    vec[27][2] = kActionReduce | 14;
    vec[27][3] = kActionReduce | 14;
    vec[27][4] = kActionReduce | 14;
    vec[27][5] = kActionReduce | 14;
    vec[27][8] = kActionReduce | 14;
    vec[28][2] = kActionShift | 18;
    vec[28][8] = kActionShift | 19;
    vec[29][8] = kActionReduce | 7;
    vec[29][10] = kActionReduce | 7;
    vec[30][2] = kActionReduce | 19;
    vec[30][3] = kActionReduce | 19;
    vec[30][4] = kActionReduce | 19;
    vec[30][5] = kActionReduce | 19;
    vec[30][8] = kActionReduce | 19;
    vec[31][2] = kActionReduce | 15;
    vec[31][3] = kActionReduce | 15;
    vec[31][4] = kActionReduce | 15;
    vec[31][5] = kActionReduce | 15;
    vec[31][8] = kActionReduce | 15;
    vec[32][4] = kActionReduce | 9;
    vec[32][5] = kActionReduce | 9;
}

void ScriptParserData::InitTokenToSymbol(std::vector<int>& vec) {
    vec[0] = 0;
    vec[kTokenSymbMark] = 1;
    vec[kTokenTerminal] = 2;
    vec[kTokenProdMark] = 3;
    vec['|'] = 4;
    vec[';'] = 5;
    vec[kTokenString] = 6;
    vec[':'] = 7;
    vec[kTokenId] = 8;
    vec[kTokenSyntaxBegin] = 9;
    vec[kTokenBlockEnd] = 10;
    vec[kTokenTokenBegin] = 11;
}

void ScriptParserData::InitProductions(std::vector<common::Production>& prods) {
    // Symbols: non-terminals
    // 0 seq
    // 1 slice
    // 2 body
    // 3 prod_priority
    // 4 token
    // 5 atom
    // 6 symbol
    // 7 prod
    // 8 syntax
    // 9 tokens
    // 10 symb_priority
    // 11 script
    // 12 ROOT

    // Productions
    prods[0] = {0, {false, 12}, {{false, 11}}};  // ROOT → script
    prods[1] = {1, {false, 11},
            {{true, 11}, {false, 9}, {true, 10}, {true, 9}, {false, 8},
                    {true, 10}}};  // script → token_begin tokens block_end syntax_begin syntax block_end
    prods[2] = {2, {false, 9}, {{false, 4}}};              // tokens → token
    prods[3] = {3, {false, 9}, {{false, 9}, {false, 4}}};  // tokens → tokens token
    prods[4] = {
            4, {false, 4}, {{true, 8}, {true, 7}, {true, 6}, {true, 5}}};  // token → id : string ;
    prods[5] = {5, {false, 8}, {{false, 7}}};                              // syntax → prod
    prods[6] = {6, {false, 8}, {{false, 8}, {false, 7}}};                  // syntax → syntax prod
    prods[7] = {
            7, {false, 7}, {{true, 8}, {true, 7}, {false, 2}, {true, 5}}};  // prod → id : body ;
    prods[8] = {8, {false, 2}, {{false, 1}}};                               // body → slice
    prods[9] = {9, {false, 2}, {{false, 2}, {true, 4}, {false, 1}}};        // body → body | slice
    prods[10] = {10, {false, 1}, {{false, 0}, {false, 3}}};   // slice → seq prod_priority
    prods[11] = {11, {false, 3}, {}};                         // prod_priority → ε
    prods[12] = {12, {false, 3}, {{true, 3}}};                // prod_priority → prod_mark
    prods[13] = {13, {false, 0}, {{false, 6}}};               // seq → symbol
    prods[14] = {14, {false, 0}, {{false, 0}, {false, 6}}};   // seq → seq symbol
    prods[15] = {15, {false, 6}, {{false, 5}, {false, 10}}};  // symbol → atom symb_priority
    prods[16] = {16, {false, 5}, {{true, 2}}};                // atom → @terminal
    prods[17] = {17, {false, 5}, {{true, 8}}};                // atom → @id
    prods[18] = {18, {false, 10}, {}};                        // symb_priority → ε
    prods[19] = {19, {false, 10}, {{true, 1}}};               // symb_priority → symb_mark
}

ScriptParserData::ScriptParserData(const std::optional<DFASetter*>& dfa_setter,
        const std::optional<LanguageSetter*>& lang_setter, bool print_debug_info)
    : dfa_setter_(dfa_setter), lang_setter_(lang_setter), print_debug_info_(print_debug_info) {
    parser_ = std::make_unique<NFARegexParser>();
    syntax_ = std::make_unique<Syntax>();
    productions_.emplace_back();
}

// ROOT -> script
// void ScriptParserData::ReduceRoot(const std::vector<std::unique_ptr<Property>>& props) {}

// script -> token_begin tokens block_end syntax_begin syntax block_end
void ScriptParserData::ReduceScript() const {
    parser_->RegisterSingles(singles_);
    DFABuilder dfa_builder(*parser_);
    dfa_builder.set_print_debug_info(print_debug_info_);
    dfa_builder.Build(dfa_setter_);
    LALRBuilder lalr_builder(*syntax_);
    lalr_builder.set_print_debug_info(print_debug_info_);
    lalr_builder.Build(lang_setter_);
}

// tokens -> token
// void ScriptParserData::ReduceTokens0(const std::vector<std::unique_ptr<Property>>& props) {}

// tokens -> tokens token
// void ScriptParserData::ReduceTokens1(const std::vector<std::unique_ptr<Property>>& props) {}

// token -> id : string ;
void ScriptParserData::ReduceToken(const std::vector<std::unique_ptr<Property>>& props) const {
    auto& t0 = static_cast<PropertyTerminal*>(props[0].get())->token->Cast<TokenId>();
    auto& t2 = static_cast<PropertyTerminal*>(props[2].get())->token->Cast<TokenStringVal>();
    auto id = t0.val;
    auto regex = t2.val;
    parser_->Register(regex, id);
}

// syntax -> prod
// void ScriptParserData::ReduceSyntax0(const std::vector<std::unique_ptr<Property>>& props) {}

// syntax -> syntax prod
// void ScriptParserData::ReduceSyntax1(const std::vector<std::unique_ptr<Property>>& props) {}

// prod -> @id : body ;
void ScriptParserData::ReduceProd(const std::vector<std::unique_ptr<Property>>& props) {
    auto& token = props[0]->Cast<PropertyTerminal>()->token->Cast<TokenId>();
    Symbol s(token.val, SymbolType::kNonTerminal);
    productions_.erase(productions_.end() - 1);
    for (auto& prod : productions_) {
        prod.head = s;
        syntax_->AddProduction(prod);
    }
    productions_.assign(1, {});
}

// body -> slice
// void ScriptParserData::ReduceBody0(const std::vector<std::unique_ptr<Property>>& props) {}

// body -> body | slice
// void ScriptParserData::ReduceBody1(const std::vector<std::unique_ptr<Property>>& props) {}

// slice -> seq prod_priority
void ScriptParserData::ReduceSlice() {
    productions_.emplace_back();
}

// prod_priority -> ε
// void ScriptParserData::ReduceProdPriority0(const std::vector<std::unique_ptr<Property>>& props) {}

// prod_priority -> prod_mark
void ScriptParserData::ReduceProdPriority1(const std::vector<std::unique_ptr<Property>>& props) {
    auto& token = props[0]->Cast<PropertyTerminal>()->token->Cast<TokenProdMark>();
    auto& prod = productions_.back();
    prod.assoc = token.assoc;
    prod.priority = token.prior;
}

// seq -> symbol
// void ScriptParserData::ReduceSeq0(const std::vector<std::unique_ptr<Property>>& props) {}

// seq -> seq symbol
// void ScriptParserData::ReduceSeq1(const std::vector<std::unique_ptr<Property>>& props) {}

// symbol -> atom symb_priority
// void ScriptParserData::ReduceSymbol(const std::vector<std::unique_ptr<Property>>& props) {}

// atom -> @terminal
void ScriptParserData::ReduceAtom0(const std::vector<std::unique_ptr<Property>>& props) {
    auto& token = props[0]->Cast<PropertyTerminal>()->token->Cast<TokenTerminal>();
    Symbol s;
    s.type = SymbolType::kTerminal;
    switch (token.type) {
        case TokenTerminal::Type::kSingle:
            singles_.insert(token.ch);
            s.name = '\'' + std::string{token.ch} + '\'';
            break;
        case TokenTerminal::Type::kEpsilon: {
            s.name = epsilon_string_name_;
            break;
        }
        case TokenTerminal::Type::kNormal:
            s.name = token.name;
            break;
        case TokenTerminal::Type::kKeyWord:
            // 暂不支持
            return;
    }
    auto& body = productions_.back().body;
    if (!body.empty() && body.back().name == epsilon_string_name_) {
        throw std::runtime_error("You cannot add symbols to an epsilon production");
    }
    body.push_back(std::move(s));
}

// atom -> @id
void ScriptParserData::ReduceAtom1(const std::vector<std::unique_ptr<Property>>& props) {
    auto& token = props[0]->Cast<PropertyTerminal>()->token->Cast<TokenId>();
    Symbol s;
    s.type = SymbolType::kNonTerminal;
    s.name = token.val;
    auto& body = productions_.back().body;
    if (!body.empty() && body.back().name == epsilon_string_name_) {
        throw std::runtime_error("You cannot add symbols to an epsilon production");
    }
    body.push_back(std::move(s));
}
// symb_priority → @~
// void ScriptParserData::ReduceSymbPriority0(const std::vector<std::unique_ptr<Property>>& props) {}

// symb_priority → @symb_mark
void ScriptParserData::ReduceSymbPriority1(const std::vector<std::unique_ptr<Property>>& props) {
    auto& token = props[0]->Cast<PropertyTerminal>()->token->Cast<TokenSymbMark>();
    auto& symbol = productions_.back().body.back();
    symbol.assoc = token.assoc;
    symbol.priority = token.prior;
}

}  // namespace cc::gen
