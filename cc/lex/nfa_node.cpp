#include "nfa_node.h"

#include <sstream>

namespace cc {

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

void NFANode::ForEachEdge(const std::function<void(NFANode* cur, NFANode* nxt)>& func) {
    switch (edge_type_) {
        case EdgeType::kSingleEpsilon:
        case EdgeType::kChar:
            func(this, next1_);
            break;
        case EdgeType::kDoubleEpsilon:
            func(this, next1_);
            func(this, next2_);
            break;
        default:
            break;
    }
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

}  // namespace cc
