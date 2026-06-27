//
// Created by PC on 2026/6/28.
//

#include "dfa_builder.h"

#include <vector>

#include "nfa_node.h"

namespace cc {

DfaBuilder::DfaBuilder(NFANode* root_node) : root_node_(root_node) {}

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
    std::array<std::vector<bool>, kMaxChars> char_signatures;
    for (int c = 0; c < kMaxChars; ++c) {
        auto& signature = char_signatures[c];
        for (const auto& pred : predicates) {
            signature.push_back(pred(c));
        }
    }
    //按签名分组，相同签名的字符归为一个类
    std::unordered_map<std::vector<bool>, int> class_map;
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

void DfaBuilder::VisitNfa(const std::function<void(NFANode*)>& processor) const {
    std::vector node_visited(NFANode::node_count(), false);
    std::vector unvisited{root_node_};
    while (!unvisited.empty()) {
        auto* node = unvisited.back();
        unvisited.pop_back();
        if (node_visited[node->id()]) {
            continue;
        }
        processor(node);
        node_visited[node->id()] = true;
        switch (node->edge_type()) {
            case NFANode::EdgeType::kSingleEpsilon:
            case NFANode::EdgeType::kChar:
                unvisited.push_back(node->next1());
                break;
            case NFANode::EdgeType::kDoubleEpsilon:
                unvisited.push_back(node->next1());
                unvisited.push_back(node->next2());
                break;
            default:
                break;
        }
    }
}

}  // namespace cc
