//
// Created by PC on 2026/7/2.
//

#ifndef CC_SCRIPT_PARSER_DATA_H
#define CC_SCRIPT_PARSER_DATA_H

#include "compile/lexer.h"
#include "compile/parser.h"
#include "lex/dfa_builder.h"
#include "lex/nfa_regex_parser.h"
#include "syntax/lalr_parser.h"

namespace cc {

struct DFASetter;
struct LanguageSetter;

}  // namespace cc

namespace cc::gen {

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

using common::CompilerInput;
using common::Token;
using common::TokenMatchResult;

struct TokenId : Token {
    std::string val;
    int Type() override;
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override;
};

struct TokenStringVal : Token {
    std::string val;
    int Type() override;
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override;
};

struct TokenTerminal : Token {
    enum class Type { kSingle, kEpsilon, kNormal, kKeyWord } type;
    std::string name;
    char ch = 0;
    int Type() override;
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override;
};

struct TokenProdMark : Token {
    Associativity assoc = Associativity::kUndefined;
    int prior = 0;
    int Type() override;
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override;
};

struct TokenSymbMark : Token {
    Associativity assoc = Associativity::kUndefined;
    int prior = 0;
    int Type() override;
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override;
};

// Keyword tokens (token_begin, syntax_begin, block_end) can use TokenSingle or dedicated structs.
// For simplicity, they are single-char/keyword markers with type = their constant value.
// The DFA accepts them as "token_begin", "syntax_begin", "block_end" → map to their types.

struct TokenTokenBegin : Token {
    int Type() override;
};

struct TokenSyntaxBegin : Token {
    int Type() override;
};

struct TokenBlockEnd : Token {
    int Type() override;
};

// ==================== Parser Data ====================

class ScriptParserData : public common::ParserDataSupplier {
public:
    explicit ScriptParserData(const std::optional<DFASetter*>& dfa_setter = std::nullopt,
            const std::optional<LanguageSetter*>& lang_setter = std::nullopt, bool print_debug_info = false);

    int TokenMappersCount() const override;
    int NonTerminalSymbolsCount() const override;
    int TerminalSymbolsCount() const override;
    int StatesCount() const override;
    int ProductionsCount() const override;
    void InitReduceActions(std::vector<common::ReduceFunc>& vec) override;
    void InitGoto(std::vector<std::vector<int>>& vec) override;
    void InitActions(std::vector<std::vector<int>>& vec) override;
    void InitTokenToSymbol(std::vector<int>& vec) override;
    void InitProductions(std::vector<common::Production>& productions) override;

private:
    std::optional<DFASetter*> dfa_setter_;
    std::optional<LanguageSetter*> lang_setter_;
    std::unique_ptr<NFARegexParser> parser_;
    std::unique_ptr<Syntax> syntax_;

    std::vector<Production> productions_;
    std::unordered_set<char> singles_;
    bool print_debug_info_;

    const std::string epsilon_string_name_{'\000'};

    //root → script
    // void ReduceRoot(const std::vector<std::unique_ptr<Token>>& props);
    //script → @token_begin tokens @block_end @syntax_begin syntax @block_end
    void ReduceScript() const;
    // tokens → token
    // void ReduceTokens0(const std::vector<std::unique_ptr<Token>>& props);
    // tokens → tokens token
    // void ReduceTokens1(const std::vector<std::unique_ptr<Token>>& props);
    // token → @id ':' @string ';'
    void ReduceToken(const std::vector<std::unique_ptr<Token>>& props) const;
    // syntax → prod
    // void ReduceSyntax0(const std::vector<std::unique_ptr<Token>>& props);
    // syntax → syntax prod
    // void ReduceSyntax1(const std::vector<std::unique_ptr<Token>>& props);
    // prod → @id ':' body ';'
    void ReduceProd(const std::vector<std::unique_ptr<Token>>& props);
    // body → slice
    // void ReduceBody0(const std::vector<std::unique_ptr<Token>>& props);
    // body → body '|' slice
    // void ReduceBody1(const std::vector<std::unique_ptr<Token>>& props);
    // slice → seq prod_priority
    void ReduceSlice();
    // prod_priority → @~
    // void ReduceProdPriority0(const std::vector<std::unique_ptr<Token>>& props);
    // prod_priority → @prod_mark
    void ReduceProdPriority1(const std::vector<std::unique_ptr<Token>>& props);
    // seq → symbol
    // void ReduceSeq0(const std::vector<std::unique_ptr<Token>>& props);
    // seq → seq symbol
    // void ReduceSeq1(const std::vector<std::unique_ptr<Token>>& props);
    // symbol -> atom symb_priority
    // void ReduceSymbol(const std::vector<std::unique_ptr<Token>>& props);
    // atom -> @terminal
    void ReduceAtom0(const std::vector<std::unique_ptr<Token>>& props);
    // atom -> @id
    void ReduceAtom1(const std::vector<std::unique_ptr<Token>>& props);
    // symb_priority → @~
    // void ReduceSymbPriority0(const std::vector<std::unique_ptr<Token>>& props);
    // symb_priority → @symb_mark
    void ReduceSymbPriority1(const std::vector<std::unique_ptr<Token>>& props);
};

}  // namespace cc::gen

#endif  //CC_SCRIPT_PARSER_DATA_H
