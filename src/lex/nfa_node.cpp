#include "nfa_node.h"

#include <sstream>

namespace cc {

int NFANode::node_count_ = 0;

NFANode::NFANode() : id_(node_count_++) {}

void NFANode::AddEpsilonEdge(NFANode* next) {
    if (edge_type_ == EdgeType::kNoEdge) {
        next1_ = next;
        edge_type_ = EdgeType::kSingleEpsilon;
    } else if (edge_type_ == EdgeType::kSingleEpsilon) {
        next2_ = next;
        edge_type_ = EdgeType::kDoubleEpsilon;
    }
}

void NFANode::AddEpsilonEdge(NFANode* next1, NFANode* next2) {
    next1_ = next1;
    next2_ = next2;
    edge_type_ = EdgeType::kDoubleEpsilon;
}

void NFANode::AddCharEdge(CharPredicate predicate, NFANode* next) {
    predicate_ = std::move(predicate);
    next1_ = next;
    edge_type_ = EdgeType::kChar;
}

std::string NFANode::ToString() const {
    std::ostringstream oss;
    oss << id_ << '(';
    switch (edge_type_) {
        case EdgeType::kNoEdge:
            oss << "null";
            break;
        case EdgeType::kSingleEpsilon:
            oss << "e" << next1_->id();
            break;
        case EdgeType::kDoubleEpsilon:
            oss << "e" << next1_->id() << ",e" << next2_->id();
            break;
        case EdgeType::kChar:
            oss << "c" << next1_->id();
            break;
    }
    oss << ')';
    return oss.str();
}

}

