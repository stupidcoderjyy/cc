#include "nfa.h"

#include <sstream>
#include <utility>

namespace cc {

bool NFA::IsEmpty() const {
    return start_ == nullptr || end_ == nullptr;
}

void NFA::And(NFA other) {
    if (IsEmpty()) {
        start_ = other.start_;
        end_ = other.end_;
        nodes_ = std::move(other.nodes_);
    } else {
        end_->AddEpsilonEdge(other.start_);
        end_ = other.end_;
        MergeInternalNodes(other);
    }
}

void NFA::AndAtom(CharPredicate predicate) {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddCharEdge(std::move(predicate), new_end.get());
    if (IsEmpty()) {
        start_ = new_start.get();
        end_ = new_end.get();
    } else {
        end_->AddEpsilonEdge(new_start.get());
        end_ = new_end.get();
    }
    nodes_.push_back(std::move(new_start));
    nodes_.push_back(std::move(new_end));
}

void NFA::Star() {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddEpsilonEdge(start_, new_end.get());
    end_->AddEpsilonEdge(start_, new_end.get());
    start_ = new_start.get();
    end_ = new_end.get();
    nodes_.push_back(std::move(new_start));
    nodes_.push_back(std::move(new_end));
}

void NFA::Quest() {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddEpsilonEdge(start_, new_end.get());
    end_->AddEpsilonEdge(new_end.get());
    start_ = new_start.get();
    end_ = new_end.get();
    nodes_.push_back(std::move(new_start));
    nodes_.push_back(std::move(new_end));
}

void NFA::Plus() {
    auto new_start = std::make_unique<NFANode>();
    auto new_end = std::make_unique<NFANode>();
    new_start->AddEpsilonEdge(start_);
    end_->AddEpsilonEdge(start_, new_end.get());
    start_ = new_start.get();
    end_ = new_end.get();
    nodes_.push_back(std::move(new_start));
    nodes_.push_back(std::move(new_end));
}

void NFA::Or(NFA other) {
    if (IsEmpty()) {
        start_ = other.start_;
        end_ = other.end_;
        nodes_ = std::move(other.nodes_);
    } else {
        auto new_start = std::make_unique<NFANode>();
        auto new_end = std::make_unique<NFANode>();
        new_start->AddEpsilonEdge(start_, other.start_);
        end_->AddEpsilonEdge(new_end.get());
        other.end_->AddEpsilonEdge(new_end.get());
        start_ = new_start.get();
        end_ = new_end.get();
        nodes_.push_back(std::move(new_start));
        nodes_.push_back(std::move(new_end));
        MergeInternalNodes(other);
    }
}

std::string NFA::ToString() const {
    if (IsEmpty()) {
        return "null";
    }
    return start_->ToString() + ", " + end_->ToString();
}

void NFA::MergeInternalNodes(NFA& other) {
    nodes_.insert(nodes_.end(),
                           std::make_move_iterator(other.nodes_.begin()),
                           std::make_move_iterator(other.nodes_.end()));
}

}  // namespace cc
