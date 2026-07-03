//
// Created by PC on 2026/6/28.
//

#include "dfa_builder.h"

#include <algorithm>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <vector>

#include "dfa_minimizer.h"
#include "nfa_node.h"
#include "nfa_regex_parser.h"
#include "util/print_util.h"

namespace cc {

DFABuilder::DFABuilder(NFARegexParser& parser)
    : DFABuilder(parser.root_node(), parser.node_id_to_token()) {}

DFABuilder::DFABuilder(NFANode* root_node, std::vector<std::string> nfa_node_to_token)
    : root_node_(root_node), nfa_node_to_token_(std::move(nfa_node_to_token)) {
    VisitNfa([this](NFANode* node) {
        if (node->id() >= nfa_nodes_.size()) {
            nfa_nodes_.resize(node->id() + 1);
        }
        nfa_nodes_[node->id()] = node;
    });
    if (print_debug_info_) {
        std::cout << "====== NFA Info ======\n";
        PrintNFA();
    }
}

void DFABuilder::Build(const std::optional<DFASetter*>& setter) {
    BuildCharClassMap();
    ComputeNfaCharClassMask();
    auto dfa = BuildDfaFromNfa();
    if (disable_dfa_minimize_) {
        dfa_ = std::move(dfa);
    } else {
        dfa_ = MinimizeDfa(dfa);
    }
    if (setter.has_value()) {
        OutputData(*setter.value());
    }
}

void DFABuilder::BuildCharClassMap() {
    // 收集所有CHAR边上的谓词
    std::vector<CharPredicate> predicates;
    VisitNfa([&predicates](NFANode* node) {
        if (node->edge_type() == NFANode::EdgeType::kChar) {
            predicates.push_back(node->predicate());
        }
    });
    //遍历所有候选字符，为每个字符构建签名向量
    // char_signatures[字符] = 签名向量
    if (predicates.size() > kMaxPredicates) {
        throw std::runtime_error("Too many predicates");
    }
    //char_signatures[字符][谓词]=是否符合
    std::array<std::bitset<kMaxPredicates>, kMaxChars> char_signatures;
    for (int c = 0; c < kMaxChars; ++c) {
        auto& signature = char_signatures[c];
        for (int i = 0; i < predicates.size(); ++i) {
            signature.set(i, predicates[i](c));
        }
    }
    //按签名分组，相同签名的字符归为一个类
    BitsetMap<int, kMaxPredicates> class_map;
    std::vector<int> char_to_class(kMaxChars);
    for (int c = 0; c < kMaxChars; ++c) {
        const auto& sig = char_signatures[c];
        if (!class_map.contains(sig)) {
            class_map[sig] = static_cast<int>(class_map.size());
        }
        char_to_class[c] = class_map[sig];
    }
    char_to_class_ = std::move(char_to_class);
    class_count_ = static_cast<int>(class_map.size());

    // 打印调试信息
    if (print_debug_info_) {
        std::cout << "====== Char Class Info ======\n";
        PrintCharToClass();
    }

    // 预计算字符类代表字符
    class_representative_char_.resize(class_count_);
    for (int cid = 0; cid < class_count_; ++cid) {
        for (char c = 0; c < kMaxChars; ++c) {
            if (char_to_class_[c] == cid) {
                class_representative_char_[cid] = c;
                break;
            }
        }
    }
}

void DFABuilder::ComputeNfaCharClassMask() {
    if (class_count_ > kMaxCharClass) {
        throw std::runtime_error("too many character classes");
    }
    nfa_node_to_char_class_.resize(nfa_nodes_.size());
    VisitNfa([this](NFANode* node) {
        if (node->edge_type() != NFANode::EdgeType::kChar) {
            return;  // ε边不处理
        }
        CharClassBitMask mask;
        const auto& pred = node->predicate();

        // 遍历所有字符类，判断该类是否能通过该谓词
        for (int cid = 0; cid < class_count_; ++cid) {
            // 用代表字符测试谓词
            mask.set(cid, pred(class_representative_char_[cid]));
        }

        // 保存映射
        nfa_node_to_char_class_[node->id()] = mask;
    });

    if (print_debug_info_) {
        std::cout << "====== NFANode To Class Info ======\n";
        PrintNFAToClass();
    }
}

NFAGroup DFABuilder::EpsilonClosure(NFAGroup group) {
    if (epsilon_cache_.contains(group)) {
        return epsilon_cache_[group];
    }
    // 必然能够在不输入字符的情况下到达自己
    auto result = group;
    shared_stack_.reserve(kMaxNfaGroups);
    shared_stack_.clear();
    VisitNfaGroup(group, [&](NFANode* node) { shared_stack_.push_back(node->id()); });
    auto step_epsilon = [&](NFANode* cur, NFANode* nxt) {
        // 如果后继结点不在组内，则加入组，并加入待检查集合
        if (cur->edge_type() != NFANode::EdgeType::kChar && !result.test(nxt->id())) {
            result.set(nxt->id());
            shared_stack_.push_back(nxt->id());
        }
    };
    while (!shared_stack_.empty()) {
        auto* node = nfa_nodes_[shared_stack_.back()];
        shared_stack_.pop_back();
        node->ForEachEdge(step_epsilon);
    }
    epsilon_cache_[group] = result;
    return result;
}

NFAGroup DFABuilder::Next(int char_class, NFAGroup group) {
    NFAGroup result;
    VisitNfaGroup(group, [&](NFANode* node) {
        if (node->edge_type() != NFANode::EdgeType::kChar) {
            return;
        }
        if (nfa_node_to_char_class_[node->id()].test(char_class)) {
            result.set(node->next1()->id());
        }
    });
    return EpsilonClosure(result);
}

DFA DFABuilder::BuildDfaFromNfa() {
    DFA dfa;
    std::stack<DFAState*> unprocessed;
    NFAGroupMap<DFAState::id_t> nfa_group_to_dfa_state;

    {
        // 创建初始DFA状态，并加入待处理
        NFAGroup start_nfa_group{};
        start_nfa_group.set(root_node_->id(), true);
        start_nfa_group = EpsilonClosure(start_nfa_group);
        auto* start_dfa_state = CreateDfaState(dfa, start_nfa_group);
        unprocessed.push(start_dfa_state);
        nfa_group_to_dfa_state[start_nfa_group] = start_dfa_state->id;
        dfa.start_state = start_dfa_state->id;
    }

    while (!unprocessed.empty()) {
        auto* dfa_state = unprocessed.top();
        unprocessed.pop();
        NFAGroup next_nfa_group;
        for (int cid = 0; cid < class_count_; ++cid) {
            next_nfa_group = Next(cid, dfa_state->nfa_group);
            if (!next_nfa_group.none()) {
                DFAState::id_t next_id;
                if (nfa_group_to_dfa_state.contains(next_nfa_group)) {
                    next_id = nfa_group_to_dfa_state[next_nfa_group];
                } else {
                    auto* next_dfa_state = CreateDfaState(dfa, next_nfa_group);
                    unprocessed.push(next_dfa_state);
                    next_id = next_dfa_state->id;
                    nfa_group_to_dfa_state[next_nfa_group] = next_id;
                }
                dfa_state->class_id_to_next[cid] = next_id;
            }
        }
    }
    return dfa;
}

DFA DFABuilder::MinimizeDfa(DFA& dfa) const {
    return DFAMinimizer(dfa, class_count_).Minimize();
}

void DFABuilder::VisitNfa(const std::function<void(NFANode*)>& processor) const {
    std::vector<bool> node_visited;
    node_visited.reserve(kMaxNfaGroups);
    std::vector unvisited{root_node_};
    auto add_unvisited = [&unvisited](NFANode*, NFANode* node) { unvisited.push_back(node); };
    while (!unvisited.empty()) {
        auto* node = unvisited.back();
        unvisited.pop_back();
        if (node->id() < 0) {
            throw std::runtime_error("unregistered node");
        }
        if (node->id() >= node_visited.size()) {
            node_visited.resize(node->id() + 1);
            node_visited[node->id()] = false;
        }
        if (node_visited[node->id()]) {
            continue;
        }
        processor(node);
        node_visited[node->id()] = true;
        node->ForEachEdge(add_unvisited);
    }
}

void DFABuilder::VisitNfaGroup(
        const NFAGroup& group, const std::function<void(NFANode*)>& processor) const {
    for (std::size_t i = 0; i < group.size(); ++i) {
        if (group.test(i)) {
            processor(nfa_nodes_[i]);
        }
    }
}

DFAState* DFABuilder::CreateDfaState(DFA& dfa, NFAGroup group) const {
    auto new_id = static_cast<DFAState::id_t>(dfa.states.size());

    bool accepted = false;
    std::string token{};
    std::vector<std::string> matched_tokens;  // 收集所有匹配的token类型

    VisitNfaGroup(group, [&](NFANode* node) {
        if (node->accepted()) {
            accepted = true;
            matched_tokens.push_back(nfa_node_to_token_[node->id()]);
            // 按优先级选择第一个（或保留原有逻辑，可改为按定义顺序）
            if (token.empty()) {
                token = nfa_node_to_token_[node->id()];
            }
        }
    });

    auto dfa_state = std::make_unique<DFAState>(new_id, group, accepted, token);

    // 如果匹配多个不同token，输出警告
    if (print_conflict_info_ && matched_tokens.size() > 1) {
        WarnTokenMatchConflict(*dfa_state, matched_tokens);
    }
    dfa_state->class_id_to_next.resize(class_count_, -1);
    dfa.states.push_back(std::move(dfa_state));
    return dfa.states.back().get();
}

void DFABuilder::OutputData(DFASetter& setter) const {
    if (print_debug_info_) {
        std::cout << "====== DFA Constants Info ======\n";
        std::cout << "class count = " << class_count_ << '\n';
        std::cout << "states count = " << dfa_.states.size() << '\n';
        std::cout << "start state = " << dfa_.start_state << '\n';
        std::cout << std::endl;

        std::cout << "====== DFA Node Info ======\n";
        PrintDFANodes();
    }
    // 常数信息
    setter.SetCharClassCount(class_count_);
    setter.SetDfaStatesCount(static_cast<int>(dfa_.states.size()));
    setter.SetStartState(dfa_.start_state);

    // 输出字符到字符类的映射
    for (int ch = 0; ch < kMaxChars; ++ch) {
        if (int class_id = char_to_class_[ch]; class_id > 0) {
            setter.SetCharToClass(ch, class_id);
        }
    }

    if (print_debug_info_) {
        std::cout << "====== DFA Goto Map ======\n";
    }

    // 遍历所有 DFA 状态
    for (const auto& state_ptr : dfa_.states) {
        const DFAState& state = *state_ptr;

        // 设置状态接受信息
        setter.SetStateInfo(state.id, state.is_accepted, state.token);

        // ===== 新增：打印状态信息 =====
        if (print_debug_info_) {
            PrintState(state.id, state.is_accepted);
        }

        // 遍历每个字符类，输出转移
        for (int cid = 0; cid < class_count_; ++cid) {
            if (int target = state.class_id_to_next[cid]; target != -1) {
                setter.SetGoto(state.id, cid, target);

                // ===== 新增：打印转移信息 =====
                if (print_debug_info_) {
                    PrintGoto(cid, target, dfa_.states[target]->is_accepted);
                }
            }
        }

        if (print_debug_info_) {
            std::cout << std::endl;  // 状态间空行
        }
    }

    setter.Finish();
}

void DFABuilder::PrintNFA() const {
    for (const auto& nfa_node : nfa_nodes_) {
        std::cout << nfa_node->ToString() << '\n';
    }
    std::cout << std::endl;
}

void DFABuilder::PrintNFAToClass() const {
    for (int i = 0; i < nfa_node_to_char_class_.size(); ++i) {
        std::cout << i << " → ";
        for (int c = 0; c < class_count_; ++c) {
            if (nfa_node_to_char_class_[i].test(c)) {
                std::cout << c << ' ';
            }
        }
        std::cout << '\n';
    }
    std::cout << std::endl;
}

void DFABuilder::PrintCharToClass() const {
    std::vector<std::vector<int>> class_to_char;
    class_to_char.resize(class_count_, {});

    for (int c = 0; c < kMaxChars; ++c) {
        class_to_char[char_to_class_[c]].push_back(c);
    }
    for (int clazz = 0; clazz < class_count_; ++clazz) {
        std::cout << std::left << clazz << " → ";
        for (const auto& ch : class_to_char[clazz]) {
            std::cout << DisplayChar(ch) << " ";
        }
        std::cout << '\n';
    }
    std::cout << std::endl;
}

void DFABuilder::PrintDFANodes() const {
    for (int i = 0; i < dfa_.states.size(); ++i) {
        std::cout << i << ": ";
        const auto& s = dfa_.states[i];
        for (int n = 0; n < nfa_nodes_.size(); ++n) {
            if (!s->nfa_group.test(n)) {
                continue;
            }
            std::cout << nfa_nodes_[n]->ToString() << ' ';
        }
        std::cout << '\n';
    }
    std::cout << std::endl;
}

void DFABuilder::PrintState(int state, bool accept) {
    if (accept) {
        PrintHighlightBlue("ACCEPTED", std::cout);
    } else {
        PrintHighlightYellow("NON ACCEPTED", std::cout);
    }
    std::cout << " " << state << std::endl;
}

void DFABuilder::PrintGoto(int input_class, int target, bool target_accept) {
    std::cout << std::left << std::setw(3) << input_class;
    std::cout << "→ ";
    // 目标状态显示
    if (target_accept) {
        PrintHighlightWhite(" " + std::to_string(target) + " ", std::cout);
    } else {
        std::cout << " " << target << " ";
    }
    std::cout << std::endl;
}

void DFABuilder::WarnTokenMatchConflict(DFAState& state, std::vector<std::string>& tokens) {
    Begin(0, BLACK, BG_RED, std::cerr);
    std::cerr << "MULTIPLE MATCHES";
    End(std::cerr);
    std::cerr << " state " << state.id << " matches multiple tokens: ";
    for (const auto& t : tokens)
        std::cerr << t << " ";
    std::cerr << ". Using: " << state.token << std::endl;
}

}  // namespace cc
