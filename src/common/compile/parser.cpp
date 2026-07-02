//
// Created by PC on 2026/7/1.
//

#include "parser.h"

#include "compiler_input.h"
#include "lexer.h"

namespace common {

Parser::Parser(ParserDataSupplier& core)
    : states_count_(core.StatesCount()),
      token_to_symbol_(std::vector(core.TokenMappersCount(), 0)),
      actions_(std::vector(core.StatesCount(), std::vector(core.TerminalSymbolsCount(), 0))),
      goto_(std::vector(core.StatesCount(), std::vector(core.NonTerminalSymbolsCount(), 0))),
      suppliers_(std::vector<PropertySupplier>(core.NonTerminalSymbolsCount())),
      reduce_actions_(core.NonTerminalSymbolsCount(), nullptr) {
    core.InitActions(actions_);
    core.InitGoto(goto_);
    core.InitTokenToSymbol(token_to_symbol_);
    core.InitPropertySuppliers(suppliers_);
    core.InitSymbolsAndProductions(symbols_, productions_);
    core.InitReduceActions(reduce_actions_);
}

void Parser::Parse(Lexer& lexer, CompilerInput& ci) {
    ci.Mark();
    std::vector<int> states;
    std::vector<std::unique_ptr<Property>> properties;
    states.push_back(0);
    auto token = lexer.NextToken(ci);
    if (token && token->Type() == 0) {
        return;  // EOF
    }
    while (true) {
        int s = states.back();
        int order = token ? actions_[s][token_to_symbol_[token->Type()]] : 0;
        int type = order >> 16;
        int target = order & 0xFFFF;
        switch (type) {
            default: {
                ci.Recover(false);
                OnFailed(ci, token);
                return;
            }
            case kActionAccept: {
                std::vector<std::unique_ptr<Property>> body;
                body.push_back(std::move(properties.back()));
                properties.pop_back();
                try {
                    auto& p = productions_[0];
                    if (auto& f = reduce_actions_[p.head.id]) {
                        f(body);
                    }
                } catch (std::exception& err) {
                    ci.Recover(false);
                    throw ci.ErrorAtMark(err.what());
                }
                return;
            }
            case kActionShift: {
                ci.Mark();
                states.push_back(target);
                properties.push_back(std::make_unique<PropertyTerminal>(std::move(token)));
                token = lexer.NextToken(ci);
                break;
            }
            case kActionReduce: {
                auto& p = productions_[target];
                std::vector<std::unique_ptr<Property>> body(p.body.size());
                for (int i = static_cast<int>(p.body.size()) - 1; i >= 0; i--) {
                    auto& [is_terminal, id] = p.body[i];
                    if (id < 0) {
                        body[i] = nullptr;
                        continue;  //ε
                    }
                    states.pop_back();
                    body[i] = std::move(properties.back());
                    properties.pop_back();
                    if (is_terminal) {
                        ci.RemoveMark();
                    }
                }
                try {
                    if (auto& f = reduce_actions_[p.head.id]) {
                        f(body);
                    }
                } catch (std::exception& err) {
                    ci.Recover(false);
                    throw ci.ErrorAtMark(err.what());
                }
                properties.push_back(suppliers_[p.head.id]());
                states.push_back(goto_[states.back()][p.head.id]);
                break;
            }
        }
    }
}

void Parser::OnFailed(CompilerInput& ci, const std::unique_ptr<Token>& at) {
    throw at ? ci.ErrorAtMark("syntax error") : ci.ErrorMarkToForward("unknown symbol");
}

}  // namespace common
