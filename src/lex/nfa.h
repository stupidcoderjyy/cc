#ifndef CC_NFA_H_
#define CC_NFA_H_

#include <memory>
#include <string>
#include <vector>

#include "nfa_node.h"

namespace cc {

class NFA {
public:
    NFA() = default;

    NFA(NFA&&) = default;
    NFA& operator=(NFA&&) = default;

    NFA(const NFA&) = delete;
    NFA& operator=(const NFA&) = delete;

    bool IsEmpty() const;

    void And(NFA other);
    void AndAtom(CharPredicate predicate);
    void Star();
    void Quest();
    void Plus();
    void Or(NFA other);

    std::string ToString() const;

    NFANode* start() const { return start_.get(); }
    NFANode* end() const { return end_.get(); }

private:
    std::unique_ptr<NFANode> start_;
    std::unique_ptr<NFANode> end_;
    std::vector<std::unique_ptr<NFANode>> internal_nodes_;

    void MergeInternalNodes(NFA& other);
};

}  // namespace cc
#endif  // CC_NFA_H_
