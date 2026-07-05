//
// Created by PC on 2026/7/5.
//

#include <cmath>
#include <iostream>
#include <stack>

#include "compile/compiler_input.h"
#include "gen/calculator_parser_data.h"

using common::Property;

struct TokenNumber : cal::gen::TokenNumber {
    double val_ = 0.0;
    common::TokenMatchResult OnMatched(const std::string &lexeme, common::CompilerInput &ci) override {
        val_ = std::stod(lexeme);
        return common::TokenMatchResult::kAccept;
    }
};

struct CaculatorLexerData : cal::gen::CalculatorLexerData {
    void InitTokenSuppliers(std::vector<common::TokenSupplier> &vec) override {
        vec[3] = [] { return std::make_unique<TokenNumber>(); };
        vec[2] = [] { return std::make_unique<common::TokenSingle>(); };
        vec[1] = [] { return std::make_unique<TokenNumber>(); };
    }
};

struct CaculatorParserData : cal::gen::CalculatorParserData {
    CaculatorParserData(): CalculatorParserData() {}
protected:
    // ROOT → expr
    void ReduceROOT(const std::vector<std::unique_ptr<Property>>& props) override {
        std::cout << stack_.top() << std::endl;
    }
    // expr → expr '+' expr
    void ReduceExpr0(const std::vector<std::unique_ptr<Property>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 + v1);
    }
    // expr → expr '-' expr
    void ReduceExpr1(const std::vector<std::unique_ptr<Property>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 - v1);
    }
    // expr → expr '*' expr
    void ReduceExpr2(const std::vector<std::unique_ptr<Property>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 * v1);
    }
    // expr → expr '/' expr
    void ReduceExpr3(const std::vector<std::unique_ptr<Property>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 / v1);
    }
    // expr → expr '^' expr
    void ReduceExpr4(const std::vector<std::unique_ptr<Property>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(std::pow(v2, v1));
    }
    // expr → '-' expr
    void ReduceExpr5(const std::vector<std::unique_ptr<Property>>& props) override {
        double v = stack_.top(); stack_.pop();
        stack_.push(-v);
    }
    // expr → '(' expr ')'
    void ReduceExpr6(const std::vector<std::unique_ptr<Property>>& props) override {}
    // expr → @number
    void ReduceExpr7(const std::vector<std::unique_ptr<Property>>& props) override {
        stack_.push(props[0]->Cast<common::PropertyTerminal>()->token->Cast<TokenNumber>().val_);
    }
private:
    std::stack<double> stack_;
};

int main() {
    CaculatorParserData cpd;
    CaculatorLexerData cld;
    common::Parser parser(cpd);
    common::Lexer lexer(cld);
    while (true) {
        try {
            auto ci = common::CompilerInput::FromConsole();
            parser.Parse(lexer, *ci);
        } catch (common::CompileError& e) {
            std::cerr << e.FormatErrorMessage() << std::endl;
            return 1;
        }
    }
}
