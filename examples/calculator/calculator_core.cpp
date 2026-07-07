//
// Created by PC on 2026/7/7.
//

#include "calculator_core.h"

#include <cmath>
#include <format>
#include <iostream>

common::TokenMatchResult TokenNumber::OnMatched(
        const std::string& lexeme, common::CompilerInput& ci) {
    val_ = std::stod(lexeme);
    return common::TokenMatchResult::kAccept;
}

common::TokenMatchResult TokenConstant::OnMatched(
        const std::string& lexeme, common::CompilerInput& ci) {
    if (lexeme == "$E") {
        val_ = std::numbers::e;
    } else if (lexeme == "$PI") {
        val_ = std::numbers::pi;
    } else {
        return common::TokenMatchResult::kReject;
    }
    return common::TokenMatchResult::kAccept;
}

common::TokenMatchResult TokenId::OnMatched(const std::string& lexeme, common::CompilerInput& ci) {
    val_ = lexeme;
    return common::TokenMatchResult::kAccept;
}

void CalculatorLexerCore::InitTokenSuppliers(std::vector<common::TokenSupplier>& vec) {
    vec[7] = [] { return std::make_unique<TokenNumber>(); };
    vec[4] = [] { return std::make_unique<common::TokenSingle>(); };
    vec[3] = [] { return std::make_unique<TokenNumber>(); };
    vec[2] = [] { return std::make_unique<TokenConstant>(); };
    vec[1] = [] { return std::make_unique<TokenId>(); };
}

CalculatorParserData::CalculatorParserData() : cal::gen::CalculatorParserData() {
    functions_["sin"] = {[](auto& stack) {
                             double v = stack.top();
                             stack.pop();
                             stack.push(std::sin(v));
                         },
            1};

    functions_["cos"] = {[](auto& stack) {
                             double v = stack.top();
                             stack.pop();
                             stack.push(std::cos(v));
                         },
            1};

    functions_["tan"] = {[](auto& stack) {
                             double v = stack.top();
                             stack.pop();
                             stack.push(std::tan(v));
                         },
            1};

    functions_["exp"] = {[](auto& stack) {
                             double v = stack.top();
                             stack.pop();
                             stack.push(std::exp(v));
                         },
            1};

    functions_["ln"] = {[](auto& stack) {
                            double v = stack.top();
                            stack.pop();
                            stack.push(std::log(v));
                        },
            1};

    functions_["sqrt"] = {[](auto& stack) {
                              double v = stack.top();
                              stack.pop();
                              stack.push(std::sqrt(v));
                          },
            1};

    functions_["abs"] = {[](std::stack<double>& stack) {
                             double v = stack.top();
                             stack.pop();
                             stack.push(std::abs(v));
                         },
            1};

    functions_["floor"] = {[](auto& stack) {
                               double v = stack.top();
                               stack.pop();
                               stack.push(std::floor(v));
                           },
            1};

    functions_["ceil"] = {[](auto& stack) {
                              double v = stack.top();
                              stack.pop();
                              stack.push(std::ceil(v));
                          },
            1};

    functions_["round"] = {[](auto& stack) {
                               double v = stack.top();
                               stack.pop();
                               stack.push(std::round(v));
                           },
            1};
}

// ROOT → stmt
void CalculatorParserData::ReduceRoot(const std::vector<std::unique_ptr<Token>>& props) {
    if (!cur_var_.empty()) {
        variables_[cur_var_] = stack_.top();
        stack_.pop();
        std::cout << variables_[cur_var_] << std::endl;
    }
}

// stmt → @id '=' expr
void CalculatorParserData::ReduceStmt0(const std::vector<std::unique_ptr<Token>>& props) {
    cur_var_ = props[0]->Cast<TokenId>().val_;
}

// stmt → expr
void CalculatorParserData::ReduceStmt1(const std::vector<std::unique_ptr<Token>>& props) {
    cur_var_ = "ans";
}

// expr → expr '+' expr
void CalculatorParserData::ReduceExpr0(const std::vector<std::unique_ptr<Token>>& props) {
    double v1 = stack_.top();
    stack_.pop();
    double v2 = stack_.top();
    stack_.pop();
    stack_.push(v2 + v1);
}

// expr → expr '-' expr
void CalculatorParserData::ReduceExpr1(const std::vector<std::unique_ptr<Token>>& props) {
    double v1 = stack_.top();
    stack_.pop();
    double v2 = stack_.top();
    stack_.pop();
    stack_.push(v2 - v1);
}

// expr → expr '*' expr
void CalculatorParserData::ReduceExpr2(const std::vector<std::unique_ptr<Token>>& props) {
    double v1 = stack_.top();
    stack_.pop();
    double v2 = stack_.top();
    stack_.pop();
    stack_.push(v2 * v1);
}

// expr → expr '/' expr
void CalculatorParserData::ReduceExpr3(const std::vector<std::unique_ptr<Token>>& props) {
    double v1 = stack_.top();
    stack_.pop();
    double v2 = stack_.top();
    stack_.pop();
    stack_.push(v2 / v1);
}

// expr → expr '^' expr
void CalculatorParserData::ReduceExpr4(const std::vector<std::unique_ptr<Token>>& props) {
    double v1 = stack_.top();
    stack_.pop();
    double v2 = stack_.top();
    stack_.pop();
    stack_.push(std::pow(v2, v1));
}

// expr → '-' expr
void CalculatorParserData::ReduceExpr5(const std::vector<std::unique_ptr<Token>>& props) {
    double v = stack_.top();
    stack_.pop();
    stack_.push(-v);
}

// expr → '(' expr ')'
void CalculatorParserData::ReduceExpr6(const std::vector<std::unique_ptr<Token>>& props) {}

// expr → @id '(' args ')'
void CalculatorParserData::ReduceExpr7(const std::vector<std::unique_ptr<Token>>& props) {
    auto& func_name = props[0]->Cast<TokenId>().val_;
    if (!functions_.contains(func_name)) {
        throw std::runtime_error("unknown function: " + func_name);
    }
    auto& [f, argc] = functions_[func_name];
    if (argc != argc_) {
        throw std::runtime_error(std::format(
                "function {} requires {} arguments, but was given {}", func_name, argc, argc_));
    }
    f(stack_);
}

// expr → @number
void CalculatorParserData::ReduceExpr8(const std::vector<std::unique_ptr<Token>>& props) {
    stack_.push(props[0]->Cast<TokenNumber>().val_);
}

// expr → @id
void CalculatorParserData::ReduceExpr9(const std::vector<std::unique_ptr<Token>>& props) {
    cur_var_ = props[0]->Cast<TokenId>().val_;
    stack_.push(variables_[props[0]->Cast<TokenId>().val_]);
}
// expr → @constant
void CalculatorParserData::ReduceExpr10(const std::vector<std::unique_ptr<Token>>& props) {
    stack_.push(props[0]->Cast<TokenConstant>().val_);
}

// args → arg
void CalculatorParserData::ReduceArgs0(const std::vector<std::unique_ptr<Token>>& props) {
    argc_ = 1;
}

// args → args ',' arg
void CalculatorParserData::ReduceArgs1(const std::vector<std::unique_ptr<Token>>& props) {
    argc_++;
}

// arg → expr
void CalculatorParserData::ReduceArg(const std::vector<std::unique_ptr<Token>>& props) {}
