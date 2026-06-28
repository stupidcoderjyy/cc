//
// Created by PC on 2026/6/28.
//

#include "dfa_builder.h"

#include <algorithm>
#include <bitset>
#include <stdexcept>
#include <vector>

#include "nfa_node.h"
#include "nfa_regex_parser.h"
#include "util/bitset_hash.h"

namespace cc {

DfaBuilder::DfaBuilder(NFANode* root_node) : root_node_(root_node) {
    VisitNfa([this](NFANode* node) {
        if (node->id() >= id_to_node_.size()) {
            id_to_node_.resize(node->id() + 1);
        }
        id_to_node_[node->id()] = node;
    });
}

void DfaBuilder::Build() {
    BuildCharClassMap();
}

void DfaBuilder::BuildCharClassMap() {
    // 1. 收集所有CHAR边上的谓词
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
    std::unordered_map<std::bitset<kMaxPredicates>, int, BitsetHash<kMaxPredicates>> class_map;
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
}

DfaBuilder::NfaGroup DfaBuilder::EpsilonClosure(NfaGroup group) {
    if (epsilon_cache_.contains(group)) {
        return epsilon_cache_[group];
    }
    // 必然能够在不输入字符的情况下到达自己
    auto result = group;
    std::vector<int> nodes;
    nodes.reserve(kMaxNfaGroups);
    for (int i = 0; i < id_to_node_.size(); ++i) {
        if (group.test(i)) {
            nodes.push_back(i);
        }
    }
    auto step_epsilon = [&](NFANode* cur, NFANode* nxt) {
        // 如果后继结点不在组内，则加入组，并加入待检查集合
        if (cur->edge_type() != NFANode::EdgeType::kChar && !result.test(nxt->id())) {
            result.set(nxt->id());
            nodes.push_back(nxt->id());
        }
    };
    while (!nodes.empty()) {
        auto* node = id_to_node_[nodes.back()];
        nodes.pop_back();
        node->ForEachEdge(step_epsilon);
    }
    epsilon_cache_[group] = result;
    return result;
}

void DfaBuilder::VisitNfa(const std::function<void(NFANode*)>& processor) const {
    std::vector<bool> node_visited;
    node_visited.reserve(kMaxNfaGroups);
    std::vector unvisited{ root_node_ };
    auto add_unvisited = [&unvisited](NFANode*, NFANode* node) {
        unvisited.push_back(node);
    };
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

}  // namespace cc
