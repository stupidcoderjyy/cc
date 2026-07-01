//
// Created by PC on 2026/6/30.
//

#ifndef CC_LEXER_H
#define CC_LEXER_H
#include <functional>
#include <memory>

namespace common {

class CompilerInput;

enum class TokenMatchResult { kAccept, kPass, kReject };

struct Token {
    virtual int Type() = 0;
    virtual TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) {
        return TokenMatchResult::kAccept;
    }
    virtual ~Token() = default;
    template <class T>
    T& Cast() {
        return dynamic_cast<T&>(*this);
    }
};

typedef std::function<std::unique_ptr<Token>()> TokenSupplier;

struct LexerDataSupplier {
    virtual int CharClassCount() = 0;
    virtual int StatesCount() = 0;
    virtual int StartState() = 0;
    virtual void InitAccepted(std::vector<bool>& vec) = 0;
    virtual void InitGoto(std::vector<std::vector<int>>& vec) = 0;
    virtual void InitCharToClass(std::vector<int>& vec) = 0;
    virtual void InitTokenSuppliers(std::vector<TokenSupplier>& vec) = 0;
    virtual ~LexerDataSupplier() = default;
};

class Lexer {
public:
    explicit Lexer(LexerDataSupplier& lds);
    std::unique_ptr<Token> NextToken(CompilerInput& ci);

protected:
    int char_class_count_;
    int states_count_;
    int start_state_;
    std::vector<bool> accepted_;
    std::vector<std::vector<int>> goto_;
    std::vector<int> char_to_class_;
    std::vector<TokenSupplier> token_suppliers_;
};

struct TokenEof : Token {
    int Type() override { return 0; }
};

struct TokenSingle : Token {
    char ch;
    int Type() override { return ch; }
    TokenMatchResult OnMatched(const std::string& lexeme, CompilerInput& ci) override {
        ch = lexeme[0];
        return TokenMatchResult::kAccept;
    }
};

}  // namespace common

#endif  //CC_LEXER_H
