//
// Generated Parser file by stupidcoder cc
// Date: 2026-07-06 06:03
//

#ifndef CC_CALCULATOR_PARSER_DATA_H
#define CC_CALCULATOR_PARSER_DATA_H

#include "compile/lexer.h"
#include "compile/parser.h"

namespace cc {

struct DFASetter;
struct LanguageSetter;

}  // namespace cc

namespace cal::gen {

// ==================== Token Types ====================

constexpr int kTokenNumber = 127;

// ==================== Lexer Data ====================

struct CalculatorLexerData : common::LexerDataSupplier {
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
using common::TokenSingle;

struct TokenNumber : Token {
    int Type() override;
};

// ==================== Property ====================

using common::Property;
using common::PropertyTerminal;

// ==================== Parser Data ====================

class CalculatorParserData : public common::ParserDataSupplier {
public:
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

protected:
    // ROOT → expr
    virtual void ReduceRoot(const std::vector<std::unique_ptr<Property>>& props);
    // expr → expr '+' expr
    virtual void ReduceExpr0(const std::vector<std::unique_ptr<Property>>& props);
    // expr → expr '-' expr
    virtual void ReduceExpr1(const std::vector<std::unique_ptr<Property>>& props);
    // expr → expr '*' expr
    virtual void ReduceExpr2(const std::vector<std::unique_ptr<Property>>& props);
    // expr → expr '/' expr
    virtual void ReduceExpr3(const std::vector<std::unique_ptr<Property>>& props);
    // expr → expr '^' expr
    virtual void ReduceExpr4(const std::vector<std::unique_ptr<Property>>& props);
    // expr → '-' expr
    virtual void ReduceExpr5(const std::vector<std::unique_ptr<Property>>& props);
    // expr → '(' expr ')'
    virtual void ReduceExpr6(const std::vector<std::unique_ptr<Property>>& props);
    // expr → @number
    virtual void ReduceExpr7(const std::vector<std::unique_ptr<Property>>& props);
};

}  // namespace cal::gen

#endif  //CC_CALCULATOR_PARSER_DATA_H
