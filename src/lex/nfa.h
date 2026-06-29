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

    NFANode* start() const { return start_; }
    NFANode* end() const { return end_; }
    std::vector<std::unique_ptr<NFANode>>& nodes() { return nodes_; }

private:
    NFANode* start_ = nullptr;
    NFANode* end_ = nullptr;
    // 持有结点所有权
    std::vector<std::unique_ptr<NFANode>> nodes_;

    void MergeInternalNodes(NFA& other);
};

}  // namespace cc
#endif  // CC_NFA_H_
