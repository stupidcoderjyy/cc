//
// Created by PC on 2026/7/1.
//

#ifndef CC_PARSER_H
#define CC_PARSER_H
#include <functional>
#include <memory>
#include <vector>

#include "lexer.h"

namespace common {

struct Symbol final {
    bool is_terminal;
    int id;
};

struct Production final {
    int id;
    Symbol head;
    std::vector<Symbol> body;
};

struct Property {};

struct PropertyTerminal : Property {
    explicit PropertyTerminal(std::unique_ptr<Token> token) : token(std::move(token)) {}
    std::unique_ptr<Token> token;
};

class CompilerInput;
class Lexer;

typedef std::function<std::unique_ptr<Property>()> PropertySupplier;
typedef std::function<void(const std::vector<std::unique_ptr<Property>>&)> ReduceFunc;

struct ParserDataSupplier {
    virtual int TokenMappersCount() const = 0;
    virtual int NonTerminalSymbolsCount() const = 0;
    virtual int TerminalSymbolsCount() const = 0;
    virtual int StatesCount() const = 0;
    virtual void InitReduceActions(std::vector<ReduceFunc>& vec) = 0;
    virtual void InitGoto(std::vector<std::vector<int>>& vec) = 0;
    virtual void InitActions(std::vector<std::vector<int>>& vec) = 0;
    virtual void InitTokenToSymbol(std::vector<int>& vec) = 0;
    virtual void InitPropertySuppliers(std::vector<PropertySupplier>& vec) = 0;
    virtual void InitSymbolsAndProductions(
            std::vector<Symbol>& symbols, std::vector<Production>& productions) = 0;
    virtual ~ParserDataSupplier() = default;
};

enum ParserAction { kActionAccept = 1 << 16, kActionShift = 2 << 16, kActionReduce = 3 << 16 };

class Parser {
public:
    explicit Parser(ParserDataSupplier& core);
    void Parse(Lexer& lexer, CompilerInput& ci);
    virtual ~Parser() = default;

protected:
    int states_count_;
    std::vector<int> token_to_symbol_;
    std::vector<std::vector<int>> actions_;
    std::vector<std::vector<int>> goto_;
    std::vector<Production> productions_;
    std::vector<PropertySupplier> suppliers_;
    std::vector<Symbol> symbols_;
    std::vector<ReduceFunc> reduce_actions_;
    virtual void OnFailed(CompilerInput& ci, const std::unique_ptr<Token>& at);
};

}  // namespace common

#endif  //CC_PARSER_H
