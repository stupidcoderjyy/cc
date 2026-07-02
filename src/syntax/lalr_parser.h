//
// Created by PC on 2026/6/30.
//

#ifndef CC_LALR_PARSER_H
#define CC_LALR_PARSER_H
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "cc_constants.h"
#include "lalr_conflict_handler.h"
#include "language_setter.h"
#include "syntax.h"
#include "util/bitset_hash.h"

namespace cc {

struct Item {
    int prod_id;
    int dot_pos;

    bool operator==(const Item&) const = default;
    bool operator<(const Item& other) const {
        if (prod_id != other.prod_id) return prod_id < other.prod_id;
        return dot_pos < other.dot_pos;
    }
};

struct LR0State {
    std::set<Item> items;
    std::map<int, int> transitions;
};

class LALRBuilder {
public:
    explicit LALRBuilder(Syntax& syntax,
                         const std::optional<LanguageSetter*>& setter = std::nullopt);
    ~LALRBuilder();

    // --- 设置冲突处理器 ---
    void SetConflictHandler(std::unique_ptr<LALRConflictHandler> handler);

    // --- FIRST 集 (Visible for Testing) ---
    const SymbolSet& First(int symbolId) const { return symbol_to_first_set_[symbolId]; }
    bool HasEpsilon(int symbolId) const { return symbol_to_has_epsilon_[symbolId]; }

    // --- LR(0) 闭包与 GOTO (Visible for Testing) ---
    std::set<Item> Closure(std::set<Item> items) const;
    std::set<Item> GotoFunc(const std::set<Item>& items, int symbolId) const;

    // --- LR(0) 项集族 (Visible for Testing) ---
    void BuildCanonicalCollection();
    const std::vector<LR0State>& lr0_states() const { return lr0_states_; }

    // --- FOLLOW 集 (Visible for Testing) ---
    void ComputeFollowSets();
    const SymbolSet& Follow(int symbolId) const { return symbol_to_follow_set_[symbolId]; }

    // --- LALR 合并 (Visible for Testing) ---
    void MergeLALRStates();
    const std::vector<LR0State>& lalr_states() const { return lalr_states_; }
    const std::vector<int>& lr0_to_lalr() const { return lr0_to_lalr_; }

    // --- 前瞻符传播 (Visible for Testing) ---
    void PropagateLookaheads();
    using StateLookaheads = std::map<Item, SymbolSet>;
    const std::vector<StateLookaheads>& lr0_lookaheads() const { return lr0_lookaheads_; }

    // --- 分析表生成 (Visible for Testing) ---
    void BuildParsingTable();
    const std::vector<std::vector<Action>>& action() const { return action_; }
    const std::vector<std::map<int, int>>& gotoT() const { return goto_; }

    // --- Parse 调试专用 ---
    bool DebugParse(const std::vector<Symbol>& input) const;

    int IdOf(const Symbol& sym) const { return symbol_to_id_.at(sym); }
    const Symbol& SymbolOf(int id) const { return id_to_symbol_[id]; }
    int SymbolCount() const { return static_cast<int>(id_to_symbol_.size()); }

private:
    Syntax* syntax_;
    // 符号编号映射
    std::vector<Symbol> id_to_symbol_;
    UnorderedSymbolMap<int> symbol_to_id_;
    // FIRST 集结果
    std::vector<SymbolSet> symbol_to_first_set_;
    std::vector<bool> symbol_to_has_epsilon_;
    // FOLLOW 集结果
    std::vector<SymbolSet> symbol_to_follow_set_;
    // 非终结符 → 其产生式ID列表
    std::vector<std::vector<int>> symbol_to_productions_;
    // LR(0) 项集族
    std::vector<LR0State> lr0_states_;
    // LALR 合并后状态
    std::vector<LR0State> lalr_states_;
    std::vector<int> lr0_to_lalr_;
    // 前瞻符：每个 LR(0) 状态中每项的 lookahead 集合
    std::vector<StateLookaheads> lr0_lookaheads_;
    std::vector<std::vector<Action>> action_;
    std::vector<std::map<int, int>> goto_;

    std::unique_ptr<LALRConflictHandler> conflict_handler_;

    void InitSymbols();
    void BuildProductionIndex();
    void ComputeFirstSets();
    void OutputData(LanguageSetter& setter) const;
    std::pair<int, Associativity> LookaheadProperties(int symbolId) const;
};

}  // namespace cc

#endif  //CC_LALR_PARSER_H
