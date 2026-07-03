//
// Created by PC on 2026/6/30.
//

#include "lalr_parser.h"

#include <format>
#include <iomanip>
#include <queue>
#include <ranges>

#include "util/print_util.h"

namespace cc {

std::string Action::ToString() const {
    std::string at;
    switch (type) {
        case ActionType::kUndefined:
            at = "UNDEFINED";
            break;
        case ActionType::kShift:
            at = "SHIFT";
            break;
        case ActionType::kReduce:
            at = "REDUCE";
            break;
        case ActionType::kAccept:
            at = "ACCEPT";
            break;
    }
    return std::format("action:{}, target:{}", at, target);
}

LALRBuilder::LALRBuilder(Syntax& syntax) : syntax_(&syntax) {}

void LALRBuilder::Build(const std::optional<LanguageSetter*>& setter) {
    if (syntax_->productions().empty()) {
        throw std::runtime_error("empty production");
    }
    InitSymbols();
    BuildProductionIndex();
    ComputeFirstSets();
    BuildCanonicalCollection();
    ComputeFollowSets();
    PropagateLookaheads();
    BuildParsingTable();
    if (setter.has_value()) {
        OutputData(*setter.value());
    }
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

    add_sym(syntax_->end_symbol());
    for (const auto& s : syntax_->terminals() | std::views::values) {
        add_sym(s);
    }
    for (const auto& s : syntax_->non_terminals() | std::views::values) {
        add_sym(s);
    }
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
    if (print_debug_info_) {
        for (int s = 0; s < lr0_states_.size(); ++s) {
            PrintGroupInfo(s, lr0_states_[s].items);
        }
    }
    EmitActionShiftAndGoto();

    for (int lr = 0; lr < lr0_states_.size(); ++lr) {
        for (const auto& [item, f_symbols] : lr0_lookaheads_[lr]) {
            if (const auto& prod = syntax_->productions()[item.prod_id];
                    item.dot_pos != prod.body.size()) {
                continue;
            }
            for (const auto& f_symbol : f_symbols) {
                EmitActionReduce(lr, item, f_symbol);
            }
        }
    }
}

void LALRBuilder::EmitActionShiftAndGoto() {
    int num_lalr = static_cast<int>(lr0_states_.size());
    int num_symbols = static_cast<int>(id_to_symbol_.size());

    action_.assign(num_lalr, std::vector<Action>(num_symbols));
    goto_.resize(num_lalr);

    for (int lalr_s = 0; lalr_s < num_lalr; ++lalr_s) {
        for (const auto& [sym_id, target] : lr0_states_[lalr_s].transitions) {
            if (const auto& sym = id_to_symbol_[sym_id];
                    sym.type == SymbolType::kTerminal || sym.type == SymbolType::kEof) {
                action_[lalr_s][sym_id] = {ActionType::kShift, target};
                if (print_debug_info_) {
                    PrintShift(sym, target);
                }
            } else {
                goto_[lalr_s][sym_id] = target;
                if (print_debug_info_) {
                    PrintGoto(sym, target);
                }
            }
        }
    }
}

void LALRBuilder::EmitActionReduce(int lr_state, const Item& item, const Symbol& forward) {
    int sym_id = symbol_to_id_.at(forward);

    // 根产生式
    if (item.prod_id == 0) {
        auto& cell = action_[lr_state][sym_id];
        if (cell.type != ActionType::kUndefined) {
            throw std::runtime_error("ACCEPT conflict in state " + std::to_string(lr_state));
        }
        cell = {ActionType::kAccept, -1};
        if (print_debug_info_) PrintAccept(forward);
        return;
    }
    const auto& prod = syntax_->productions()[item.prod_id];
    auto [sym_priority, sym_assoc] = LookaheadProperties(sym_id);

    switch (auto& cell = action_[lr_state][sym_id]; cell.type) {
        case ActionType::kUndefined: {
            cell = {ActionType::kReduce, item.prod_id};
            if (print_debug_info_) PrintReduce(forward, prod);
            break;
        }
        case ActionType::kShift: {
            // Shift-Reduce Conflict
            // 比较产生式优先级与前瞻符号优先级
            if (prod.priority > sym_priority) {
                // 产生式优先级更高 → 规约
                cell = {ActionType::kReduce, item.prod_id};
                if (print_debug_info_) PrintReduce(forward, prod);
            } else if (prod.priority < sym_priority) {
                // 前瞻符号优先级更高 → 保持移进（不变）
                // cell 已经是 Shift，无需修改
            } else {
                // 优先级相等，根据结合性决策
                if (prod.assoc == Associativity::kLeft) {
                    cell = {ActionType::kReduce, item.prod_id};
                    if (print_debug_info_) PrintReduce(forward, prod);
                } else if (prod.assoc == Associativity::kRight) {
                    // 保持移进（不变）
                } else {
                    // 失败
                    cell = {ActionType::kUndefined, -1};
                }
            }
            if (print_conflict_info_) {
                WarnConflictSR(lr_state, item, forward, cell);
            }
            if (cell.type == ActionType::kUndefined) {
                throw std::runtime_error("unhandled Shift-Reduce Conflict");
            }
            break;
        }
        case ActionType::kReduce: {
            // Reduce-Reduce Conflict
            const auto& existing_prod = syntax_->productions()[cell.target];
            const auto& new_prod = prod;

            int chosen_id;
            if (existing_prod.priority > new_prod.priority) {
                chosen_id = existing_prod.id;
            } else if (existing_prod.priority < new_prod.priority) {
                chosen_id = new_prod.id;
            } else {
                // 优先级相同，选择 ID 更小的
                // TODO 报错
                chosen_id = std::min(existing_prod.id, new_prod.id);
            }
            cell = {ActionType::kReduce, chosen_id};
            if (print_debug_info_) PrintReduce(forward, syntax_->productions()[chosen_id]);
            if (print_conflict_info_) {
                WarnConflictRR(existing_prod, prod, forward, cell);
            }
            break;
        }
        case ActionType::kAccept: {
            // Accept动作具有最高优先级，忽略Reduce
            return;
        }
        default: {
            throw std::runtime_error(std::format("Unexpected action type in state {}, action: {}",
                    lr_state, static_cast<int>(cell.type)));
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
    for (int lr0_s = 0; lr0_s < static_cast<int>(lr0_states_.size()); ++lr0_s) {
        setter.SetLALRState(lr0_s, lr0_states_[lr0_s].items);
        setter.SetStateLookaheads(lr0_s, lr0_lookaheads_[lr0_s]);
    }

    // 分析表
    for (int s = 0; s < static_cast<int>(action_.size()); ++s) {
        for (int sym = 0; sym < static_cast<int>(action_[s].size()); ++sym) {
            if (const auto& [type, target] = action_[s][sym]; type != ActionType::kUndefined) {
                setter.SetAction(s, sym, static_cast<int>(type), target);
            }
        }
        for (const auto& [sym, target] : goto_[s]) {
            setter.SetGoto(s, sym - static_cast<int>(terms.size()) - 1, target);
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

std::string LALRBuilder::ItemString(int state, const Item& item) const {
    const auto& prod = syntax_->productions()[item.prod_id];
    std::string result = "<";
    result += prod.head.name;
    result += " →";

    int curPos = 0;
    int bodySize = static_cast<int>(prod.body.size());

    while (curPos < bodySize) {
        if (curPos == item.dot_pos) {
            result += " ·";
        }
        result += " ";
        result += prod.body[curPos].name;
        ++curPos;
    }
    if (item.dot_pos == curPos) {
        result += " ·";
    }

    // 获取前瞻符集合
    if (auto it = lr0_lookaheads_[state].find(item); it != lr0_lookaheads_[state].end()) {
        result += ", {";
        bool first = true;
        for (const auto& sym : it->second) {
            if (!first) result += ", ";
            first = false;
            result += sym.name;
        }
        result += "}";
    } else {
        result += ", {}";
    }
    result += ">";

    return result;
}

void LALRBuilder::WarnConflictSR(
        int state, const Item& item, const Symbol& forward, const Action& action) const {
    Begin(0, BLACK, action.type == ActionType::kUndefined ? BG_BLACK : BG_YELLOW, std::cerr);
    std::cerr << std::left << "shift-reduce conflict:";
    std::cerr << "prod:" << std::setw(30) << ItemString(state, item);
    std::cerr << "forward:'" << std::setw(13) << forward.name << "'";
    std::cerr << action.ToString();
    End(std::cerr);
    std::cerr << std::endl;
}

void LALRBuilder::WarnConflictRR(const Production& pre,
        const Production& cur,
        const Symbol& forward,
        const Action& action) const {
    Begin(0, BLACK, action.type == ActionType::kUndefined ? BG_BLACK : BG_YELLOW, std::cerr);
    std::cerr << std::left << "reduce-reduce conflict:    ";
    std::cerr << "prod1:" << std::setw(20)
              << syntax_->productions()[pre.id].ToString();  // 可调用toString或打印id
    std::cerr << "prod2:" << std::setw(20) << syntax_->productions()[cur.id].ToString();
    std::cerr << "forward: " << std::setw(13) << forward.name;
    std::cerr << action.ToString();
    End(std::cerr);
    std::cerr << std::endl;
}

void LALRBuilder::PrintGroupInfo(int groupId, const std::set<Item>& items) const {
    std::cout << "\r\ngroup " << groupId << ":" << std::endl;
    for (const auto& item : items) {
        PrintPurple(ItemString(groupId, item), std::cout);
        std::cout << std::endl;
    }
}

void LALRBuilder::PrintReduce(const Symbol& forward, const Production& prod) {
    PrintHighlightBlue("REDUCE", std::cout);
    std::cout << "        forward:";
    PrintGreen(forward.name, std::cout);
    if (int len = 22 - static_cast<int>(forward.name.length()); len > 0)
        std::cout << std::string(len, ' ');
    std::cout << "prod:";
    // 可打印更详细，此处仅用id
    PrintPurple(std::to_string(prod.id), std::cout);
    std::cout << std::endl;
}

void LALRBuilder::PrintGoto(const Symbol& input, int nextState) {
    PrintHighlightWhite("GOTO", std::cout);
    std::cout << "          input:";
    PrintGreen(input.name, std::cout);
    int len = 24 - static_cast<int>(input.name.length());
    if (len > 0) std::cout << std::string(len, ' ');
    std::cout << "to:";
    PrintPurple(std::to_string(nextState), std::cout);
    std::cout << std::endl;
}

void LALRBuilder::PrintShift(const Symbol& input, int nextState) {
    PrintHighlightYellow("SHIFT", std::cout);
    std::cout << "         input:";
    PrintGreen(input.name, std::cout);
    int len = 24 - static_cast<int>(input.name.length());
    if (len > 0) std::cout << std::string(len, ' ');
    std::cout << "next:";
    PrintPurple(std::to_string(nextState), std::cout);
    std::cout << std::endl;
}

void LALRBuilder::PrintAccept(const Symbol& forward) {
    PrintHighlightPurple("ACCEPT", std::cout);
    std::cout << "        forward:";
    PrintGreen(forward.name, std::cout);
    std::cout << std::endl;
}

}  // namespace cc
