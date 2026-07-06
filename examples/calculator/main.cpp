//
// Created by PC on 2026/7/5.
//

#include <cmath>
#include <iostream>
#include <stack>

#include "compile/compiler_input.h"
#include "calculator_parser_data.h"

using common::Token;

struct TokenNumber : cal::gen::TokenNumber {
    double val_ = 0.0;
    common::TokenMatchResult OnMatched(const std::string &lexeme, common::CompilerInput &ci) override {
        val_ = std::stod(lexeme);
        return common::TokenMatchResult::kAccept;
    }
};

struct TokenId : cal::gen::TokenId {
    std::string val_;
    common::TokenMatchResult OnMatched(const std::string &lexeme, common::CompilerInput &ci) override {
        val_ = lexeme;
        return common::TokenMatchResult::kAccept;
    }
};

struct CaculatorLexerData : cal::gen::CalculatorLexerData {
    void InitTokenSuppliers(std::vector<common::TokenSupplier> &vec) override {
        vec[4] = [] { return std::make_unique<TokenNumber>(); };
        vec[3] = [] { return std::make_unique<common::TokenSingle>(); };
        vec[2] = [] { return std::make_unique<TokenNumber>(); };
        vec[1] = [] { return std::make_unique<TokenId>(); };
    }
};

struct CaculatorParserData : cal::gen::CalculatorParserData {
    CaculatorParserData(): CalculatorParserData() {}
protected:
    // ROOT → stmt
    void ReduceRoot(const std::vector<std::unique_ptr<Token>>& props) override {
        if (!cur_var_.empty()) {
            variables_[cur_var_] = stack_.top(); stack_.pop();
            std::cout << cur_var_ << " = " << variables_[cur_var_] << std::endl;
        }
    }
    // stmt → @id '=' expr
    void ReduceStmt0(const std::vector<std::unique_ptr<Token>>& props) override {
        cur_var_ = props[0]->Cast<TokenId>().val_;
    }
    // stmt → expr
    void ReduceStmt1(const std::vector<std::unique_ptr<Token>>& props) override {
        cur_var_ = "ans";
    }
    // expr → expr '+' expr
    void ReduceExpr0(const std::vector<std::unique_ptr<Token>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 + v1);
    }
    // expr → expr '-' expr
    void ReduceExpr1(const std::vector<std::unique_ptr<Token>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 - v1);
    }
    // expr → expr '*' expr
    void ReduceExpr2(const std::vector<std::unique_ptr<Token>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 * v1);
    }
    // expr → expr '/' expr
    void ReduceExpr3(const std::vector<std::unique_ptr<Token>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(v2 / v1);
    }
    // expr → expr '^' expr
    void ReduceExpr4(const std::vector<std::unique_ptr<Token>>& props) override {
        double v1 = stack_.top(); stack_.pop();
        double v2 = stack_.top(); stack_.pop();
        stack_.push(std::pow(v2, v1));
    }
    // expr → '-' expr
    void ReduceExpr5(const std::vector<std::unique_ptr<Token>>& props) override {
        double v = stack_.top(); stack_.pop();
        stack_.push(-v);
    }
    // expr → '(' expr ')'
    void ReduceExpr6(const std::vector<std::unique_ptr<Token>>& props) override {}
    // expr → @number
    void ReduceExpr7(const std::vector<std::unique_ptr<Token>>& props) override {
        stack_.push(props[0]->Cast<TokenNumber>().val_);
    }
    // expr → @id
    void ReduceExpr8(const std::vector<std::unique_ptr<Token>>& props) override {
        cur_var_ = props[0]->Cast<TokenId>().val_;
        stack_.push(variables_[cur_var_]);
    }
private:
    std::stack<double> stack_;
    std::unordered_map<std::string, double> variables_;
    std::string cur_var_;
};

int main() {
    CaculatorParserData cpd;
    CaculatorLexerData cld;
    common::Parser parser(cpd);
    common::Lexer lexer(cld);
    while (true) {
        try {
            std::cout << "> ";
            auto ci = common::CompilerInput::FromConsole();
            parser.Parse(lexer, *ci);
        } catch (common::CompileError& e) {
            std::cerr << e.FormatErrorMessage() << std::endl;
        }
    }
}
