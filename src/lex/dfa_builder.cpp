//
// Created by PC on 2026/6/28.
//

#include "dfa_builder.h"

#include <algorithm>
#include <bitset>
#include <stack>
#include <stdexcept>
#include <vector>

#include "dfa_minimizer.h"
#include "nfa_node.h"
#include "nfa_regex_parser.h"

namespace cc {

DfaBuilder::DfaBuilder(NFANode* root_node, std::vector<std::string> nfa_node_to_token)
    : root_node_(root_node), nfa_node_to_token_(std::move(nfa_node_to_token)) {
    VisitNfa([this](NFANode* node) {
        if (node->id() >= nfa_nodes_.size()) {
            nfa_nodes_.resize(node->id() + 1);
        }
        nfa_nodes_[node->id()] = node;
    });
}

void DfaBuilder::Build(DfaSetter& setter) {
    BuildCharClassMap();
    ComputeNfaCharClassMask();
    auto dfa = BuildDfaFromNfa();
    auto minimized = MinimizeDfa(dfa);
    OutputData(minimized, setter);
}

void DfaBuilder::BuildCharClassMap() {
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

void DfaBuilder::ComputeNfaCharClassMask() {
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
}

NfaGroup DfaBuilder::EpsilonClosure(NfaGroup group) {
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

NfaGroup DfaBuilder::Next(int char_class, NfaGroup group) {
    NfaGroup result;
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

Dfa DfaBuilder::BuildDfaFromNfa() {
    Dfa dfa;
    std::stack<DfaState*> unprocessed;
    NfaGroupMap<DfaState::id_t> nfa_group_to_dfa_state;

    {
        // 创建初始DFA状态，并加入待处理
        NfaGroup start_nfa_group{};
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
        NfaGroup next_nfa_group;
        for (int cid = 0; cid < class_count_; ++cid) {
            next_nfa_group = Next(cid, dfa_state->nfa_group);
            if (!next_nfa_group.none()) {
                DfaState::id_t next_id;
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

Dfa DfaBuilder::MinimizeDfa(Dfa& dfa) const {
    return DfaMinimizer(dfa, class_count_).Minimize();
}

void DfaBuilder::VisitNfa(const std::function<void(NFANode*)>& processor) const {
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

void DfaBuilder::VisitNfaGroup(const NfaGroup& group,
                               const std::function<void(NFANode*)>& processor) const {
    for (std::size_t i = 0; i < group.size(); ++i) {
        if (group.test(i)) {
            processor(nfa_nodes_[i]);
        }
    }
}

DfaState* DfaBuilder::CreateDfaState(Dfa& dfa, NfaGroup group) const {
    auto new_id = static_cast<DfaState::id_t>(dfa.states.size());

    bool accepted = false;
    std::string token{};
    // 判断是否是终结结点
    VisitNfaGroup(group, [&](NFANode* node) {
        if (node->accepted()) {
            accepted = true;
            token = nfa_node_to_token_[node->id()];
        }
    });
    // 创建结点
    auto dfa_state = std::make_unique<DfaState>(new_id, group, accepted, token);
    dfa_state->class_id_to_next.resize(class_count_, -1);  //初始化为失败状态
    dfa.states.push_back(std::move(dfa_state));
    return dfa.states.back().get();
}

void DfaBuilder::OutputData(Dfa& dfa, DfaSetter& setter) const {
    // 1. 字符类信息（必须输出）
    setter.SetCharClassCount(class_count_);
    setter.SetCharToClass(char_to_class_);  // char_to_class_ 大小为 kMaxChars

    // 2. DFA 基本结构
    setter.SetDfaStatesCount(static_cast<int>(dfa.states.size()));
    setter.SetStartState(dfa.start_state);

    // 3. 遍历所有状态
    for (const auto& state_ptr : dfa.states) {
        const DfaState& state = *state_ptr;

        // 设置状态接受信息
        setter.SetStateInfo(state.id, state.is_accepted, state.token);

        // 遍历每个字符类，输出转移
        for (int cid = 0; cid < class_count_; ++cid) {
            // 仅当有转移时调用（-1 表示无转移，可根据 setter 设计选择是否输出）
            if (int target = state.class_id_to_next[cid]; target != -1) {
                setter.SetTransition(state.id, cid, target);
            }
        }
    }

    // 4. 可选收尾
    setter.Finish();
}

}  // namespace cc
