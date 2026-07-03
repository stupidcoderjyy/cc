//
// Created by PC on 2026/6/30.
//

#ifndef CC_LALR_PARSER_H
#define CC_LALR_PARSER_H
#include <format>
#include <map>
#include <optional>
#include <set>
#include <vector>

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

enum class ActionType { kUndefined = 0, kShift = 1, kReduce = 2, kAccept = 3 };

struct Action {
    ActionType type;
    int target;
    std::string ToString() const;
};

class LALRBuilder {
public:
    explicit LALRBuilder(Syntax& syntax);
    ~LALRBuilder() = default;

    void Build(const std::optional<LanguageSetter*>& setter = std::nullopt);

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

    // --- 前瞻符传播 (Visible for Testing) ---
    void PropagateLookaheads();
    using StateLookaheads = std::map<Item, SymbolSet>;
    const std::vector<StateLookaheads>& lr0_lookaheads() const { return lr0_lookaheads_; }

    // --- 分析表生成 (Visible for Testing) ---
    void BuildParsingTable();
    void EmitActionShiftAndGoto();
    void EmitActionReduce(int lr_state, const Item& item, const Symbol& forward);
    const std::vector<std::vector<Action>>& action() const { return action_; }
    const std::vector<std::map<int, int>>& gotoT() const { return goto_; }

    // --- Parse 调试专用 ---
    bool DebugParse(const std::vector<Symbol>& input) const;

    int IdOf(const Symbol& sym) const { return symbol_to_id_.at(sym); }
    const Symbol& SymbolOf(int id) const { return id_to_symbol_[id]; }
    int SymbolCount() const { return static_cast<int>(id_to_symbol_.size()); }

    void set_print_debug_info(bool debug_info) { print_debug_info_ = debug_info; }
    void set_print_conflict_info(bool show_conflict) { print_conflict_info_ = show_conflict; }

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
    // 前瞻符：每个 LR(0) 状态中每项的 lookahead 集合
    std::vector<StateLookaheads> lr0_lookaheads_;
    // 分析器在状态s输入符号x时的动作
    std::vector<std::vector<Action>> action_;
    std::vector<std::map<int, int>> goto_;

    bool print_debug_info_ = false;
    bool print_conflict_info_ = false;

    void InitSymbols();
    void BuildProductionIndex();
    void ComputeFirstSets();
    void OutputData(LanguageSetter& setter) const;
    std::pair<int, Associativity> LookaheadProperties(int symbolId) const;

    std::string ItemString(int state, const Item& item) const;
    void WarnConflictSR(
            int state, const Item& item, const Symbol& forward, const Action& action) const;
    void WarnConflictRR(const Production& pre,
            const Production& cur,
            const Symbol& forward,
            const Action& action) const;
    void PrintGroupInfo(int groupId, const std::set<Item>& items) const;
    static void PrintReduce(const Symbol& forward, const Production& prod);
    static void PrintGoto(const Symbol& input, int nextState);
    static void PrintShift(const Symbol& input, int nextState);
    static void PrintAccept(const Symbol& forward);
};

}  // namespace cc

#endif  //CC_LALR_PARSER_H
