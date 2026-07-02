//
// Created by PC on 2026/6/30.
//

#include "lalr_parser.h"

#include <queue>
#include <ranges>

namespace cc {

LALRBuilder::LALRBuilder(Syntax& syntax, const std::optional<LanguageSetter*>& setter)
    : syntax_(&syntax) {
    conflict_handler_ = std::make_unique<DefaultConflictHandler>();
    InitSymbols();
    BuildProductionIndex();
    ComputeFirstSets();
    BuildCanonicalCollection();
    MergeLALRStates();
    ComputeFollowSets();
    PropagateLookaheads();
    BuildParsingTable();
    if (setter.has_value()) {
        OutputData(*setter.value());
    }
}

LALRBuilder::~LALRBuilder() = default;

void LALRBuilder::SetConflictHandler(std::unique_ptr<LALRConflictHandler> handler) {
    conflict_handler_ = std::move(handler);
}

// 建立 Symbol → sym_id
// 建立 Symbol → List<Production>
void LALRBuilder::InitSymbols() {
    // 收集所有符号：终结符 + 非终结符 + EOF
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

// 计算 FIRST(Symbol)
// FIRST(X) = {X}, X是终结符号
//          = {ε}, X只能展开为 X → ε
//          = {α | 存在X → α Y, α是终结符号}
void LALRBuilder::ComputeFirstSets() {
    int n = static_cast<int>(id_to_symbol_.size());
    symbol_to_first_set_.assign(n, {});
    symbol_to_has_epsilon_.assign(n, false);

    // FIRST(X) = {X}
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

// 获取所有等价LR0项集
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

// GOTO(LR0Item item, id) = Closure({ [A → αX·β | A → αX·β ∈ item && X.id == id] })
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

/*
输入: 根产生式
输出: LR0状态(LR0项集 + 转移表)

算法:
// 生成初始状态并压栈
root_group = CLOSURE({ S' → ·S })
stack << REGISTER(root_group)

// 遍历所有可达状态
while stack is not empty {
    stack >> i
    group = states[i]

    // 收集当前状态中，圆点后所有可能的符号（不区分终结符/非终结符）
    symbols = group.filter(可输入).map((A → α · X β) → X).collect()

    // 遍历每一个符号，计算 GOTO 目标
    symbols.foreach(X -> {
        // 移动圆点
        moved_items = GOTO(group, X).filter(非空).collect()

        // 注册状态，计算 GOTO 目标
        next_id;
        next_group = GROUP(moved_items);
        if (EXISTS(next_group)) {
            next_id = next_group.id
        } else {
            next_id = REGISTER(next_group)
            stack << next_id
        }

        // 记录GOTO
        GOTO[i][X] = next_id
    })
}
*/
void LALRBuilder::BuildCanonicalCollection() {
    std::map<std::set<Item>, int> seen;
    std::queue<int> queue;

    // REGISTER
    auto register_lr0 = [this, &seen, &queue](const std::set<Item>& items) {
        int new_id = static_cast<int>(lr0_states_.size());
        seen.emplace(items, new_id);
        lr0_states_.push_back({items, {}});
        queue.push(new_id);
        return new_id;
    };

    register_lr0(Closure({{0, 0}}));

    while (!queue.empty()) {
        int cur = queue.front();
        queue.pop();

        // symbols = group.filter(可输入).map((A → α · X β) → X).collect()
        std::set<int> symbols;
        for (const auto& [prod_id, dot_pos] : lr0_states_[cur].items) {
            if (const auto& prod = syntax_->productions()[prod_id];
                dot_pos < static_cast<int>(prod.body.size())) {
                symbols.insert(symbol_to_id_.at(prod.body[dot_pos]));
            }
        }

        for (int sym_id : symbols) {
            //moved_items = GOTO(group, X).filter(非空).collect()
            auto next_items = GotoFunc(lr0_states_[cur].items, sym_id);
            if (next_items.empty()) continue;

            int target;
            if (auto it = seen.find(next_items); it != seen.end()) {
                target = it->second;
            } else {
                target = register_lr0(next_items);
            }
            //GOTO[i][X] = next_id
            lr0_states_[cur].transitions[sym_id] = target;
        }
    }
}
/*
输入：LR0状态(项集 + 转移表)
输出：LALR状态(项集 + 转移表)
过程：遍历 LR0 状态，用 std::map<std::set<Item>, int> 按核心分组
     同一组分配同一个新 ID。然后重构转移边，将旧状态的目标映射到合并后的目标

lalr = []
lr0_to_lalr = []
// 核心项集 -> LALR状态ID
core_to_id = [] → []

// 根据LR0项集族创建LALR项集族
lr0.foreach((i, items) -> {
    if (core_to_id.contains(items)) {
        lr0_to_lalr[i] = core_to_id[items]
    } else {
        // 记录core_to_id + 加入lalr
        lr0_to_lalr[i] = REGISTER_LALR(items, {})
    }
})

// 把LR0项集族的转移数据同步到LALR
lr0.foreach((i, items, goto) -> {
    lalr = lr0_to_lalr[i]
    goto.foreach((x, next) -> {
        lalr_next = lr0_to_lalr[next]
        if (lalr.goto NOT CONTAINS x) {
            lalr.goto[x] = lalr_next
            RETURN
        }
        if (lalr.goto[x] != lalr_next) {
            ERROR
        }
    })
})

TODO 等待删除
*/
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

/*
计算 FOLLOW(非终结符) = 非终结符右侧的第一个终结符号

FOLLOW[ROOT] = { $ }

changed = true
while (changed) {
    changed = false
    production.foreach((A -> x1 x2 ... xn) -> {
        for (i = 0; i < n; ++i) {
            if (xi.isTerminal) continue

            //β -> x(j+1) x(j+2) ... xn
            has_epsilon = true
            for (j = i + 1; j < n; ++j) {
                // 将 FIRST(xj) 中除 ε 外的符号加入 FOLLOW(xi)
                set = FIRST(xj).filter(NOT ε)
                if (set NOT EMPTY) {
                    changed = true
                    FOLLOW[xi] << set
                }

                // 判断FIRST(xj)是否包含 ε
                if (ε NOT IN FIRST(xj)) {
                    has_epsilon = false
                    break
                }
            }

            if (!has_epsilon) CONTINUE

            FOLLOW[A].filter(NOT IN FOLLOW[xi]).foreach(x -> {
                changed = true
                FOLLOW[xi] << x
            })
        }
    })
}
*/
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

/*
// 1. 初始化
lookaheads[STATE][ITEM] = [][]->{}

lookaheads[0][root_lr0] << { $ }

changed = true
while (changed) {
    changed = false

    range(0, states.size).foreach(s -> {
        snapshot = COPY lookaheads[s]
        snapshot.filter(set NOT EMPTY).foreach((item = A → α · X β, set) -> {
            prod = item.prod
            if (item 不可输入) CONTINUE

            x = 下一个符号

            // 规则 1：通过 GOTO 转移传播（Shift Propagation）
            GOTO[s].filter(CONTAINS x).foreach((?,t) -> {
                set.filter(lookaheads[t][A → α X · β] NOT CONTAINS).foreach(s -> {
                    lookaheads[t][A → α X · β] << s
                    changed = true
                })
            })

            if (x IS_TERMINAL) CONTINUE

            // B = X · β = x0 x1 ... xn
            temp_set = {}
            has_epsilon = true
            for (j = 0; j < n; ++j) {
                FIRST[xj].filter(NOT ε).collect(temp_set)
                if (FIRST[xj] NOT CONTAINS ε) {
                    has_epsilon = false
                    BREAK
                }
            }
            if (has_epsilon) {
                temp_set << set
            }

            productions.filter(head == x).foreach((x → ·γ){
                temp_set.filter(lookaheads[s][x → ·γ] NOT CONTAINS s).foreach(s0 -> {
                    lookaheads[s][x → ·γ].add(s0)
                    changed = true
                })
            })
        })
    })
}
*/
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
            if (const auto& sym = id_to_symbol_[sym_id];
                sym.type == SymbolType::kTerminal || sym.type == SymbolType::kEof) {
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

void LALRBuilder::OutputData(LanguageSetter& setter) const {
    // 产生式
    setter.SetProductions(syntax_->productions());

    // 符号表
    std::vector<Symbol> terms, non_terms;
    for (const auto& s : syntax_->terminals() | std::views::values) {
        terms.push_back(s);
    }
    for (const auto& s : syntax_->non_terminals() | std::views::values) {
        non_terms.push_back(s);
    }
    setter.SetTerminals(terms);
    setter.SetNonTerminals(non_terms);

    // LALR 状态 + 前瞻符
    for (int lalr_s = 0; lalr_s < static_cast<int>(lalr_states_.size()); ++lalr_s) {
        setter.SetLALRState(lalr_s, lalr_states_[lalr_s].items);

        // 合并该 LALR 状态对应的所有 LR(0) 状态的前瞻符
        std::map<Item, SymbolSet> merged_la;
        for (int lr0_s = 0; lr0_s < static_cast<int>(lr0_states_.size()); ++lr0_s) {
            if (lr0_to_lalr_[lr0_s] != lalr_s) continue;
            for (const auto& [item, la] : lr0_lookaheads_[lr0_s]) {
                merged_la[item].insert(la.begin(), la.end());
            }
        }
        setter.SetStateLookaheads(lalr_s, merged_la);
    }

    // 分析表
    for (int s = 0; s < static_cast<int>(action_.size()); ++s) {
        for (int sym = 0; sym < static_cast<int>(action_[s].size()); ++sym) {
            const auto& act = action_[s][sym];
            if (act.type != ActionType::kError) {
                setter.SetAction(s, sym, static_cast<int>(act.type), act.target);
            }
        }
        for (const auto& [sym, target] : goto_[s]) {
            setter.SetGoto(s, sym, target);
        }
    }

    setter.SetStartState(0);
    setter.Finish();
}

bool LALRBuilder::DebugParse(const std::vector<Symbol>& input) const {
    std::vector<int> state_stack;
    state_stack.push_back(0);  // 初始 LALR 状态

    size_t pos = 0;

    while (true) {
        int state = state_stack.back();
        int lookahead_id;

        if (pos < input.size()) {
            lookahead_id = symbol_to_id_.at(input[pos]);
        } else {
            lookahead_id = symbol_to_id_.at(syntax_->end_symbol());
        }

        if (const auto& [type, target] = action_[state][lookahead_id]; type == ActionType::kShift) {
            state_stack.push_back(target);
            ++pos;
        } else if (type == ActionType::kReduce) {
            const auto& prod = syntax_->productions()[target];
            int len = static_cast<int>(prod.body.size());
            state_stack.resize(state_stack.size() - len);
            int new_state = state_stack.back();
            int nt_id = symbol_to_id_.at(prod.head);
            auto git = goto_[new_state].find(nt_id);
            if (git == goto_[new_state].end()) {
                return false;
            }
            state_stack.push_back(git->second);
        } else if (type == ActionType::kAccept) {
            return true;
        } else {
            return false;  // ERROR
        }
    }
}

}  // namespace cc
