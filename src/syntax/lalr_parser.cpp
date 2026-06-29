//
// Created by PC on 2026/6/30.
//

#include "lalr_parser.h"

#include <queue>
#include <ranges>

namespace cc {

LALRBuilder::LALRBuilder(Syntax& syntax) : syntax_(&syntax) {
    conflict_handler_ = std::make_unique<DefaultConflictHandler>();
    InitSymbols();
    BuildProductionIndex();
    ComputeFirstSets();
    BuildCanonicalCollection();
    MergeLALRStates();
    ComputeFollowSets();
    PropagateLookaheads();
    BuildParsingTable();
}

LALRBuilder::~LALRBuilder() = default;

void LALRBuilder::SetConflictHandler(std::unique_ptr<LALRConflictHandler> handler) {
    conflict_handler_ = std::move(handler);
}

void LALRBuilder::InitSymbols() {
    auto add_sym = [this](const Symbol& s) {
        if (!symbol_to_id_.contains(s)) {
            int id = static_cast<int>(id_to_symbol_.size());
            id_to_symbol_.push_back(s);
            symbol_to_id_[s] = id;
            return id;
        }
        return symbol_to_id_[s];
    };

    for (const auto& s : syntax_->terminals() | std::views::values) {
        add_sym(s);
    }
    for (const auto& s : syntax_->non_terminals() | std::views::values) {
        add_sym(s);
    }
    add_sym(syntax_->end_symbol());
}

void LALRBuilder::BuildProductionIndex() {
    symbol_to_productions_.resize(id_to_symbol_.size());

    for (const auto& prod : syntax_->productions()) {
        int nt_id = symbol_to_id_[prod.head];
        symbol_to_productions_[nt_id].push_back(prod.id);
    }
}

void LALRBuilder::ComputeFirstSets() {
    int n = static_cast<int>(id_to_symbol_.size());
    symbol_to_first_set_.assign(n, {});
    symbol_to_has_epsilon_.assign(n, false);

    for (int i = 0; i < n; ++i) {
        if (const auto& sym = id_to_symbol_[i]; sym.type != SymbolType::kNonTerminal) {
            symbol_to_first_set_[i].insert(sym);
        }
    }

    for (const auto& prod : syntax_->productions()) {
        if (prod.body.empty()) {
            symbol_to_has_epsilon_[symbol_to_id_[prod.head]] = true;
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : syntax_->productions()) {
            int head_id = symbol_to_id_[prod.head];
            bool has_epsilon = true;

            for (const auto& s_body : prod.body) {
                int s_body_id = symbol_to_id_[s_body];
                for (const auto& s : symbol_to_first_set_[s_body_id]) {
                    if (symbol_to_first_set_[head_id].insert(s).second) {
                        changed = true;
                    }
                }
                if (!symbol_to_has_epsilon_[s_body_id]) {
                    has_epsilon = false;
                    break;
                }
            }

            if (has_epsilon && !symbol_to_has_epsilon_[head_id]) {
                symbol_to_has_epsilon_[head_id] = true;
                changed = true;
            }
        }
    }
}

std::set<Item> LALRBuilder::Closure(std::set<Item> items) const {
    auto result = std::move(items);
    std::queue<Item> worklist;
    for (const auto& it : result) {
        worklist.push(it);
    }

    while (!worklist.empty()) {
        auto [prod_id, dot_pos] = worklist.front();
        worklist.pop();

        const auto& prod = syntax_->productions()[prod_id];
        if (dot_pos >= static_cast<int>(prod.body.size())) {
            continue;
        }

        const auto& next_sym = prod.body[dot_pos];
        if (next_sym.type != SymbolType::kNonTerminal) {
            continue;
        }

        int nt_id = symbol_to_id_.at(next_sym);
        for (int p_id : symbol_to_productions_[nt_id]) {
            Item new_item{p_id, 0};
            if (auto [it_unused, inserted] = result.insert(new_item); inserted) {
                worklist.push(new_item);
            }
        }
    }

    return result;
}

std::set<Item> LALRBuilder::GotoFunc(const std::set<Item>& items, int symbolId) const {
    std::set<Item> moved;

    for (const auto& [prod_id, dot_pos] : items) {
        const auto& prod = syntax_->productions()[prod_id];
        if (dot_pos >= static_cast<int>(prod.body.size())) {
            continue;
        }
        if (symbol_to_id_.at(prod.body[dot_pos]) != symbolId) {
            continue;
        }
        moved.insert({prod_id, dot_pos + 1});
    }

    return Closure(std::move(moved));
}

void LALRBuilder::BuildCanonicalCollection() {
    std::map<std::set<Item>, int> seen;
    std::queue<int> worklist;

    std::set<Item> start_items = Closure({{0, 0}});
    lr0_states_.push_back({start_items, {}});
    seen.emplace(start_items, 0);
    worklist.push(0);

    while (!worklist.empty()) {
        int cur = worklist.front();
        worklist.pop();

        std::set<int> next_symbols;
        for (const auto& [prod_id, dot_pos] : lr0_states_[cur].items) {
            const auto& prod = syntax_->productions()[prod_id];
            if (dot_pos < static_cast<int>(prod.body.size())) {
                next_symbols.insert(symbol_to_id_.at(prod.body[dot_pos]));
            }
        }

        for (int sym_id : next_symbols) {
            auto next_items = GotoFunc(lr0_states_[cur].items, sym_id);
            if (next_items.empty()) continue;

            int target;
            if (auto it = seen.find(next_items); it != seen.end()) {
                target = it->second;
            } else {
                target = static_cast<int>(lr0_states_.size());
                seen.emplace(next_items, target);
                lr0_states_.push_back({std::move(next_items), {}});
                worklist.push(target);
            }
            lr0_states_[cur].transitions[sym_id] = target;
        }
    }
}

void LALRBuilder::MergeLALRStates() {
    int n = static_cast<int>(lr0_states_.size());
    lr0_to_lalr_.resize(n, -1);

    std::map<std::set<Item>, int> core_to_lalr;

    for (int i = 0; i < n; ++i) {
        const auto& core = lr0_states_[i].items;
        if (auto it = core_to_lalr.find(core); it != core_to_lalr.end()) {
            lr0_to_lalr_[i] = it->second;
        } else {
            int new_id = static_cast<int>(lalr_states_.size());
            lalr_states_.push_back({core, {}});
            core_to_lalr[core] = new_id;
            lr0_to_lalr_[i] = new_id;
        }
    }

    for (int i = 0; i < n; ++i) {
        int lalr_i = lr0_to_lalr_[i];
        for (const auto& [sym_id, target] : lr0_states_[i].transitions) {
            int lalr_target = lr0_to_lalr_[target];
            if (auto it = lalr_states_[lalr_i].transitions.find(sym_id);
                it != lalr_states_[lalr_i].transitions.end()) {
                if (it->second != lalr_target) {
                    throw std::runtime_error("Grammar is not LALR: conflicting GOTO after merge");
                }
            } else {
                lalr_states_[lalr_i].transitions[sym_id] = lalr_target;
            }
        }
    }
}

void LALRBuilder::ComputeFollowSets() {
    int n = static_cast<int>(id_to_symbol_.size());
    symbol_to_follow_set_.assign(n, {});

    int root_id = symbol_to_id_.at(syntax_->root_symbol());
    symbol_to_follow_set_[root_id].insert(syntax_->end_symbol());

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& prod : syntax_->productions()) {
            int head_id = symbol_to_id_[prod.head];
            const auto& body = prod.body;

            for (size_t i = 0; i < body.size(); ++i) {
                int sym_id = symbol_to_id_[body[i]];
                if (id_to_symbol_[sym_id].type != SymbolType::kNonTerminal) {
                    continue;
                }

                bool all_nullable = true;
                for (size_t j = i + 1; j < body.size() && all_nullable; ++j) {
                    int beta_id = symbol_to_id_[body[j]];
                    for (const auto& s : symbol_to_first_set_[beta_id]) {
                        if (symbol_to_follow_set_[sym_id].insert(s).second) {
                            changed = true;
                        }
                    }
                    if (!symbol_to_has_epsilon_[beta_id]) {
                        all_nullable = false;
                    }
                }

                if (all_nullable) {
                    for (const auto& s : symbol_to_follow_set_[head_id]) {
                        if (symbol_to_follow_set_[sym_id].insert(s).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

void LALRBuilder::PropagateLookaheads() {
    int n = static_cast<int>(lr0_states_.size());
    lr0_lookaheads_.resize(n);

    // 初始项 [ROOT -> ·S] 在状态0的前瞻符 = {$}
    lr0_lookaheads_[0][{0, 0}].insert(syntax_->end_symbol());

    bool changed = true;
    while (changed) {
        changed = false;

        for (int s = 0; s < n; ++s) {
            // 复制一份当前的 lookahead map，避免在迭代中修改导致 UB
            for (auto snapshot = lr0_lookaheads_[s]; const auto& [item, la] : snapshot) {
                if (la.empty()) continue;

                const auto& prod = syntax_->productions()[item.prod_id];
                if (item.dot_pos >= static_cast<int>(prod.body.size())) continue;

                const auto& next_sym = prod.body[item.dot_pos];
                int next_id = symbol_to_id_.at(next_sym);

                // 1. 沿 GOTO 传播：目标状态中对应项继承 LA
                if (auto tit = lr0_states_[s].transitions.find(next_id);
                    tit != lr0_states_[s].transitions.end()) {
                    int target = tit->second;
                    Item advanced{item.prod_id, item.dot_pos + 1};
                    for (const auto& sym : la) {
                        if (lr0_lookaheads_[target][advanced].insert(sym).second) {
                            changed = true;
                        }
                    }
                }

                // 2. 自发生成：若圆点后是非终结符，为闭包项计算前瞻符
                if (next_sym.type != SymbolType::kNonTerminal) continue;

                // 计算 FIRST(β)，β = body[dotPos+1 ... end]
                SymbolSet spontaneous;
                bool all_nullable = true;
                for (int j = item.dot_pos + 1; j < static_cast<int>(prod.body.size()); ++j) {
                    int beta_id = symbol_to_id_.at(prod.body[j]);
                    for (const auto& s0 : symbol_to_first_set_[beta_id]) {
                        spontaneous.insert(s0);
                    }
                    if (!symbol_to_has_epsilon_[beta_id]) {
                        all_nullable = false;
                        break;
                    }
                }
                // 若 β 全部可空，则 LA 也传播给闭包项
                if (all_nullable) {
                    for (const auto& sym : la) {
                        spontaneous.insert(sym);
                    }
                }

                // 将 spontaneous 写入当前状态中 B 的闭包项 [B -> ·γ]
                for (int nt_id = next_id; int p_id : symbol_to_productions_[nt_id]) {
                    Item closure_item{p_id, 0};
                    for (const auto& sym : spontaneous) {
                        if (lr0_lookaheads_[s][closure_item].insert(sym).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

std::pair<int, Associativity> LALRBuilder::LookaheadProperties(int symbolId) const {
    const auto& sym = id_to_symbol_[symbolId];
    if (auto* found = syntax_->FindSymbol(sym.name, sym.type)) {
        return {found->priority, found->assoc};
    }
    return {0, Associativity::kLeft};
}

void LALRBuilder::BuildParsingTable() {
    int num_lalr = static_cast<int>(lalr_states_.size());
    int num_symbols = static_cast<int>(id_to_symbol_.size());

    action_.assign(num_lalr, std::vector<Action>(num_symbols));
    goto_.resize(num_lalr);

    for (int lalr_s = 0; lalr_s < num_lalr; ++lalr_s) {
        for (const auto& [sym_id, target] : lalr_states_[lalr_s].transitions) {
            const auto& sym = id_to_symbol_[sym_id];
            if (sym.type == SymbolType::kTerminal || sym.type == SymbolType::kEof) {
                action_[lalr_s][sym_id] = {ActionType::kShift, target};
            } else {
                goto_[lalr_s][sym_id] = target;
            }
        }
    }

    for (int lr0_s = 0; lr0_s < static_cast<int>(lr0_states_.size()); ++lr0_s) {
        int lalr_s = lr0_to_lalr_[lr0_s];

        for (const auto& [item, la] : lr0_lookaheads_[lr0_s]) {
            if (const auto& prod = syntax_->productions()[item.prod_id];
                item.dot_pos != static_cast<int>(prod.body.size())) {
                continue;
            }
            for (const auto& lookahead_sym : la) {
                int sym_id = symbol_to_id_.at(lookahead_sym);

                if (int root_prod_id = 0; item.prod_id == root_prod_id) {
                    auto& cell = action_[lalr_s][sym_id];
                    if (cell.type != ActionType::kError) {
                        throw std::runtime_error("ACCEPT conflict in state " +
                                                 std::to_string(lalr_s));
                    }
                    cell = {ActionType::kAccept, -1};
                } else {
                    if (auto& cell = action_[lalr_s][sym_id]; cell.type == ActionType::kError) {
                        cell = {ActionType::kReduce, item.prod_id};
                    } else if (cell.type == ActionType::kShift) {
                        const auto& prod_prio = syntax_->productions()[item.prod_id];
                        auto [la_prio, la_assoc] = LookaheadProperties(sym_id);
                        auto result = conflict_handler_->HandleShiftReduce(
                            lalr_s, sym_id, cell.target, item.prod_id, prod_prio.priority,
                            prod_prio.assoc, la_prio, la_assoc);
                        if (result == ConflictAction::kReduce) {
                            cell = {ActionType::kReduce, item.prod_id};
                        } else if (result == ConflictAction::kAbort) {
                            throw std::runtime_error("Unresolved shift-reduce conflict in state " +
                                                     std::to_string(lalr_s));
                        }
                    } else if (cell.type == ActionType::kReduce) {
                        int chosen = conflict_handler_->HandleReduceReduce(
                            lalr_s, sym_id, {cell.target, item.prod_id});
                        if (chosen < 0) {
                            throw std::runtime_error("Unresolved reduce-reduce conflict in state " +
                                                     std::to_string(lalr_s));
                        }
                        cell = {ActionType::kReduce, chosen};
                    }
                }
            }
        }
    }
}

}  // namespace cc
