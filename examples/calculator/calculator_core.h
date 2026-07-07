//
// Created by PC on 2026/7/7.
//

#ifndef CC_CALCULATOR_CORE_H
#define CC_CALCULATOR_CORE_H

#include <stack>

#include "calculator_parser_data.h"

using common::Token;

struct TokenNumber : cal::gen::TokenNumber {
    double val_ = 0.0;
    common::TokenMatchResult OnMatched(
            const std::string& lexeme, common::CompilerInput& ci) override;
};

struct TokenConstant : cal::gen::TokenConstant {
    double val_ = 0.0;
    common::TokenMatchResult OnMatched(
            const std::string& lexeme, common::CompilerInput& ci) override;
};

struct TokenId : cal::gen::TokenId {
    std::string val_;
    common::TokenMatchResult OnMatched(
            const std::string& lexeme, common::CompilerInput& ci) override;
};

struct CalculatorLexerCore : cal::gen::CalculatorLexerData {
    void InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) override;
};

struct FuncInfo {
    std::function<void(std::stack<double>&)> func;
    int argc;
};

struct CalculatorParserData : cal::gen::CalculatorParserData {
    CalculatorParserData();

protected:
    // ROOT → stmt
    void ReduceRoot(const std::vector<std::unique_ptr<Token>>& props) override;
    // stmt → @id '=' expr
    void ReduceStmt0(const std::vector<std::unique_ptr<Token>>& props) override;
    // stmt → expr
    void ReduceStmt1(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → expr '+' expr
    void ReduceExpr0(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → expr '-' expr
    void ReduceExpr1(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → expr '*' expr
    void ReduceExpr2(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → expr '/' expr
    void ReduceExpr3(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → expr '^' expr
    void ReduceExpr4(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → '-' expr
    void ReduceExpr5(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → '(' expr ')'
    void ReduceExpr6(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → @id '(' args ')'
    void ReduceExpr7(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → @number
    void ReduceExpr8(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → @id
    void ReduceExpr9(const std::vector<std::unique_ptr<Token>>& props) override;
    // expr → @constant
    void ReduceExpr10(const std::vector<std::unique_ptr<Token>>& props) override;
    // args → arg
    void ReduceArgs0(const std::vector<std::unique_ptr<Token>>& props) override;
    // args → args ',' arg
    void ReduceArgs1(const std::vector<std::unique_ptr<Token>>& props) override;
    // arg → expr
    void ReduceArg(const std::vector<std::unique_ptr<Token>>& props) override;

private:
    std::stack<double> stack_;
    std::unordered_map<std::string, double> variables_;
    std::unordered_map<std::string, FuncInfo> functions_;
    int argc_ = 0;
    std::string cur_var_;
};

#endif  //CC_CALCULATOR_CORE_H
