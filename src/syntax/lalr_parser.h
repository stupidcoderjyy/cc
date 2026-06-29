//
// Created by PC on 2026/6/30.
//

#ifndef CC_LALR_PARSER_H
#define CC_LALR_PARSER_H
#include <set>
#include <vector>

#include "syntax.h"

namespace cc {

// LR(0) 项：产生式ID + 圆点位置
struct Item {
    int prodId;
    int dotPos;

    bool operator==(const Item&) const = default;
    bool operator<(const Item& other) const {
        if (prodId != other.prodId) return prodId < other.prodId;
        return dotPos < other.dotPos;
    }
};

class LALRBuilder {
public:
    explicit LALRBuilder(Syntax& syntax);

    // --- FIRST 集 ---
    void ComputeFirstSets();
    const SymbolSet& First(int symbolId) const { return symbol_to_first_set_[symbolId]; }
    bool HasEpsilon(int symbolId) const { return symbol_to_has_epsilon_[symbolId]; }

    // --- LR(0) 闭包与 GOTO (Visible for Testing) ---
    std::set<Item> Closure(std::set<Item> items) const;
    std::set<Item> GotoFunc(const std::set<Item>& items, int symbolId) const;

    // 访问接口
    int IdOf(const Symbol& sym) const { return symbol_to_id_.at(sym); }

private:
    Syntax* syntax_;
    // 符号编号映射
    std::vector<Symbol> id_to_symbol_;
    UnorderedSymbolMap<int> symbol_to_id_;
    // FIRST 集结果
    std::vector<SymbolSet> symbol_to_first_set_;
    std::vector<bool> symbol_to_has_epsilon_;
    // 非终结符 → 其产生式ID列表
    std::vector<std::vector<int>> symbol_to_productions_;

    void InitSymbols();
    void BuildProductionIndex();
};

}  // namespace cc

#endif  //CC_LALR_PARSER_H
