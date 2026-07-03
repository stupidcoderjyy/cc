//
// Created by PC on 2026/7/2.
//

#ifndef CC_SCRIPT_PARSER_DATA_H
#define CC_SCRIPT_PARSER_DATA_H

#include "compile/lexer.h"
#include "compile/parser.h"

namespace cc::generate {

// ==================== Token Types ====================
constexpr int kTokenId = 256;
constexpr int kTokenString = 257;
constexpr int kTokenTerminal = 258;
constexpr int kTokenProdMark = 259;
constexpr int kTokenSymbMark = 260;
constexpr int kTokenTokenBegin = 261;
constexpr int kTokenSyntaxBegin = 262;
constexpr int kTokenBlockEnd = 263;

// ==================== Lexer Data ====================

struct ScriptLexerData : common::LexerDataSupplier {
    int CharClassCount() override;
    int StatesCount() override;
    int StartState() override;
    void InitAccepted(std::vector<bool>& vec) override;
    void InitGoto(std::vector<std::vector<int>>& vec) override;
    void InitCharToClass(std::vector<int>& vec) override;
    void InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) override;
};

// ==================== Token Structs ====================

struct TokenId : common::Token {
    std::string val;
    int Type() override;
};

struct TokenStringVal : common::Token {
    std::string val;
    int Type() override;
};

struct TokenTerminal : common::Token {
    int Type() override;
};

struct TokenProdMark : common::Token {
    int Type() override;
};

struct TokenSymbMark : common::Token {
    int Type() override;
};

// Keyword tokens (token_begin, syntax_begin, block_end) can use TokenSingle or dedicated structs.
// For simplicity, they are single-char/keyword markers with type = their constant value.
// The DFA accepts them as "token_begin", "syntax_begin", "block_end" → map to their types.

struct TokenTokenBegin : common::Token {
    int Type() override;
};

struct TokenSyntaxBegin : common::Token {
    int Type() override;
};

struct TokenBlockEnd : common::Token {
    int Type() override;
};

// ==================== Property ====================

// Empty property for non-symbol NTs and epsilon
struct PropertyVoid : common::Property {};

// ==================== Parser Data ====================

using common::Property;

class ScriptParserData : public common::ParserDataSupplier {
public:
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
    //script → @token_begin tokens @block_end @syntax_begin syntax @block_end
    void ReduceScript(const std::vector<std::unique_ptr<Property>>& props);
    // tokens → token
    void ReduceTokens0(const std::vector<std::unique_ptr<Property>>& props);
    // tokens → tokens token
    void ReduceTokens1(const std::vector<std::unique_ptr<Property>>& props);
    // token → @id ':' @string ';'
    void ReduceToken(const std::vector<std::unique_ptr<Property>>& props);
    // syntax → prod
    void ReduceSyntax0(const std::vector<std::unique_ptr<Property>>& props);
    // syntax → syntax prod
    void ReduceSyntax1(const std::vector<std::unique_ptr<Property>>& props);
    // prod → @id ':' body ';'
    void ReduceProd(const std::vector<std::unique_ptr<Property>>& props);
    // body → slice
    void ReduceBody0(const std::vector<std::unique_ptr<Property>>& props);
    // body → body '|' slice
    void ReduceBody1(const std::vector<std::unique_ptr<Property>>& props);
    // slice → seq prod_priority
    void ReduceSlice(const std::vector<std::unique_ptr<Property>>& props);
    // prod_priority → @~
    void ReduceProdPriority0(const std::vector<std::unique_ptr<Property>>& props);
    // prod_priority → @prod_mark
    void ReduceProdPriority1(const std::vector<std::unique_ptr<Property>>& props);
    // seq → symbol
    void ReduceSeq0(const std::vector<std::unique_ptr<Property>>& props);
    // seq → seq symbol
    void ReduceSeq1(const std::vector<std::unique_ptr<Property>>& props);
    // symbol → @id
    void ReduceSymbol0(const std::vector<std::unique_ptr<Property>>& props);
    // symbol → @terminal symb_priority
    void ReduceSymbol1(const std::vector<std::unique_ptr<Property>>& props);
    // symb_priority → @~
    void ReduceSymbPriority0(const std::vector<std::unique_ptr<Property>>& props);
    // symb_priority → @symb_mark
    void ReduceSymbPriority1(const std::vector<std::unique_ptr<Property>>& props);
};

}  // namespace cc::generate

#endif  //CC_SCRIPT_PARSER_DATA_H
