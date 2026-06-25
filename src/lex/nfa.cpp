#include "nfa.h"

#include <sstream>
#include <utility>

namespace cc {

bool NFA::IsEmpty() const {
    return start_ == nullptr || end_ == nullptr;
}

void NFA::And(NFA other) {
    if (IsEmpty()) {
        start_ = std::move(other.start_);
        end_ = std::move(other.end_);
        internal_nodes_ = std::move(other.internal_nodes_);
        return;
    }
    end_->AddEpsilonEdge(other.start_.get());
    internal_nodes_.push_back(std::move(end_));
    end_ = std::move(other.end_);
    internal_nodes_.push_back(std::move(other.start_));
    MergeInternalNodes(other);
}

void NFA::AndAtom(CharPredicate predicate) {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddCharEdge(std::move(predicate), new_end.get());
    if (IsEmpty()) {
        start_ = std::move(new_start);
        end_ = std::move(new_end);
    } else {
        end_->AddEpsilonEdge(new_start.get());
        internal_nodes_.push_back(std::move(end_));
        end_ = std::move(new_end);
        internal_nodes_.push_back(std::move(new_start));
    }
}

void NFA::Star() {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddEpsilonEdge(start_.get(), new_end.get());
    end_->AddEpsilonEdge(start_.get(), new_end.get());
    internal_nodes_.push_back(std::move(start_));
    internal_nodes_.push_back(std::move(end_));
    start_ = std::move(new_start);
    end_ = std::move(new_end);
}

void NFA::Quest() {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddEpsilonEdge(start_.get(), new_end.get());
    end_->AddEpsilonEdge(new_end.get());
    internal_nodes_.push_back(std::move(start_));
    internal_nodes_.push_back(std::move(end_));
    start_ = std::move(new_start);
    end_ = std::move(new_end);
}

void NFA::Plus() {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddEpsilonEdge(start_.get());
    end_->AddEpsilonEdge(start_.get(), new_end.get());
    internal_nodes_.push_back(std::move(start_));
    internal_nodes_.push_back(std::move(end_));
    start_ = std::move(new_start);
    end_ = std::move(new_end);
}

void NFA::Or(NFA other) {
    if (IsEmpty()) {
        start_ = std::move(other.start_);
        end_ = std::move(other.end_);
        internal_nodes_ = std::move(other.internal_nodes_);
        return;
    }
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddEpsilonEdge(start_.get(), other.start_.get());
    end_->AddEpsilonEdge(new_end.get());
    other.end_->AddEpsilonEdge(new_end.get());
    internal_nodes_.push_back(std::move(start_));
    internal_nodes_.push_back(std::move(end_));
    internal_nodes_.push_back(std::move(other.start_));
    internal_nodes_.push_back(std::move(other.end_));
    start_ = std::move(new_start);
    end_ = std::move(new_end);
    MergeInternalNodes(other);
}

std::string NFA::ToString() const {
    if (IsEmpty()) {
        return "null";
    }
    return start_->ToString() + ", " + end_->ToString();
}

void NFA::MergeInternalNodes(NFA& other) {
    internal_nodes_.insert(
        internal_nodes_.end(),
        std::make_move_iterator(other.internal_nodes_.begin()),
        std::make_move_iterator(other.internal_nodes_.end()));
}

}
