//
// Created by PC on 2026/6/30.
//

#include "lexer.h"

#include "cc_constants.h"
#include "compiler_input.h"

namespace cc {

Lexer::Lexer(int char_class_count, int states_count, int start_state)
    : char_class_count_(char_class_count), states_count_(states_count), start_state_(start_state) {
    accepted_.resize(states_count, false);
    goto_.assign(states_count, std::vector(char_class_count, -1));
    token_suppliers_.resize(states_count);
    char_to_class_.resize(kMaxChars, 0);
}

std::unique_ptr<Token> Lexer::NextToken(CompilerInput& ci) {
BEGIN:
    ci.Skip(' ', '\r', '\n');
    ci.Mark();
    if (!ci.Available()) {
        return std::make_unique<TokenEof>();
    }
    int state = start_state_;
    int last_accepted = -2;
    int extraLoadedBytes = 0;
    while (ci.Available()) {
        int b = ci.Read();
        if (b < 0) {
            try {
                ci.Retract();
                ci.ReadUtf();
            } catch (std::runtime_error&) {
                return nullptr;  //不接受非UTF字符
            }
            b = 1;  //UTF字符视为一个用不到的ASCII控制字符
        }
        state = goto_[state][char_to_class_[b]];
        if (state < 0) {
            extraLoadedBytes++;
            break;
        }
        if (accepted_[state]) {
            last_accepted = state;
            extraLoadedBytes = 0;
        } else {
            extraLoadedBytes++;
        }
    }
    if (last_accepted < 0 || !token_suppliers_[last_accepted]) {
        ci.Approach('\r', ' ', '\t');
        return nullptr;
    }
    ci.Retract(extraLoadedBytes);
    ci.Mark();
    auto token = token_suppliers_[last_accepted]();
    switch (token->OnMatched(ci.Capture(), ci)) {
        case TokenMatchResult::kAccept:
            return std::move(token);
        case TokenMatchResult::kPass:
            goto BEGIN;
        default:
            return {};
    }
}

}  // namespace cc
