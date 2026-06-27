#ifndef CC_NFA_NODE_H_
#define CC_NFA_NODE_H_

#include <cstdint>
#include <functional>
#include <string>

namespace cc {

using CharPredicate = std::function<bool(int)>;

class NFANode {
public:
    enum class EdgeType : uint8_t {
        kNoEdge = 0,
        kSingleEpsilon = 1,
        kDoubleEpsilon = 2,
        kChar = 3,
    };

    NFANode();

    static int node_count() { return node_count_; }
    void AddEpsilonEdge(NFANode* next);
    void AddEpsilonEdge(NFANode* next1, NFANode* next2);
    void AddCharEdge(CharPredicate predicate, NFANode* next);

    int id() const { return id_; }
    bool accepted() const { return accepted_; }
    EdgeType edge_type() const { return edge_type_; }
    NFANode* next1() const { return next1_; }
    NFANode* next2() const { return next2_; }
    const CharPredicate& predicate() const { return predicate_; }

    void set_accepted(bool accepted) { accepted_ = accepted; }

    std::string ToString() const;

    bool operator==(const NFANode& other) const { return id_ == other.id_; }
    bool operator!=(const NFANode& other) const { return id_ != other.id_; }

private:
    static int node_count_;

    EdgeType edge_type_ = EdgeType::kNoEdge;
    NFANode* next1_ = nullptr;
    NFANode* next2_ = nullptr;
    CharPredicate predicate_ = nullptr;
    bool accepted_ = false;
    const int id_;
};

}  // namespace cc

#endif  // CC_NFA_NODE_H_
