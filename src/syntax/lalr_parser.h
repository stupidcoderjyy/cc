//
// Created by PC on 2026/6/30.
//

#ifndef CC_LALR_PARSER_H
#define CC_LALR_PARSER_H
#include <vector>

#include "syntax.h"

namespace cc {

class LALRBuilder {
public:
    explicit LALRBuilder(Syntax& syntax);

    // 计算所有符号的 FIRST 集（不动点迭代）
    void ComputeFirstSets();

    // 访问接口
    const SymbolSet& First(int symbolId) const { return symbol_to_first_set_[symbolId]; }
    bool HasEpsilon(int symbolId) const { return symbol_to_has_epsilon_[symbolId]; }
    int IdOf(const Symbol& sym) const { return symbol_to_id_.at(sym); }

private:
    Syntax* syntax_;
    // 符号编号映射
    std::vector<Symbol> id_to_symbol_;
    UnorderedSymbolMap<int> symbol_to_id_;
    // FIRST 集结果
    std::vector<SymbolSet> symbol_to_first_set_;
    std::vector<bool> symbol_to_has_epsilon_;

    void InitSymbols();
};

}  // namespace cc

#endif  //CC_LALR_PARSER_H
