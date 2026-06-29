//
// Created by PC on 2026/6/28.
//

#ifndef CC_DFA_BUILDER_H
#define CC_DFA_BUILDER_H
#include <bitset>
#include <climits>
#include <functional>
#include <memory>
#include <utility>

#include "util/bitset_hash.h"

namespace cc {

static constexpr int kMaxChars = CHAR_MAX;  // 可输入的最大字符
static constexpr int kMaxPredicates = 128;  // 最大字符集数
static constexpr int kMaxNfaGroups = 128;   // 最大NFA状态组数量
static constexpr int kMaxCharClass = 64;    // 最大字符类数量
typedef std::bitset<kMaxNfaGroups> NfaGroup;
typedef std::bitset<kMaxCharClass> CharClassBitMask;

template <typename V>
using NfaGroupMap = BitsetMap<V, kMaxNfaGroups>;

struct DfaState {
    using id_t = int;
    int id;
    NfaGroup nfa_group;
    bool is_accepted = false;
    std::string token;
    std::vector<id_t> class_id_to_next{};
    DfaState(int id, const NfaGroup& group, bool is_accepted, std::string token)
        : id(id), nfa_group(group), is_accepted(is_accepted), token(std::move(token)) {}
};

struct Dfa {
    std::vector<std::unique_ptr<DfaState>> states;
    int start_state = -1;
};

class NFANode;
class NFARegexParser;

//https://chat.deepseek.com/share/u04wclbb8ulsd2yibr
class DfaBuilder {
public:
    explicit DfaBuilder(NFANode* root_node, std::vector<std::string> nfa_node_to_token);
    void Build();

    // Visible For Testing
    void BuildCharClassMap();
    void ComputeClassRepresentativeChar();
    void ComputeNfaCharClassMask();
    NfaGroup EpsilonClosure(NfaGroup group);
    NfaGroup Next(int char_class, NfaGroup group);
    Dfa BuildDfaFromNfa();
    Dfa MinimizeDfa(Dfa& dfa) const;

    // getter
    const std::vector<int>& char_to_class() const { return char_to_class_; }
    int class_count() const { return class_count_; }

private:
    NFANode* root_node_;
    std::vector<int> char_to_class_;
    int class_count_{};
    // 若nfa_node_to_char_class_[node][cid]，则NFA结点node接受cid字符类中的字符
    std::vector<CharClassBitMask> nfa_node_to_char_class_;
    std::vector<char> class_representative_char_;
    std::vector<std::string> nfa_node_to_token_;
    std::unordered_map<NfaGroup, NfaGroup> epsilon_cache_;
    std::vector<int> shared_stack_;
    // 不持有结点所有权
    std::vector<NFANode*> nfa_nodes_;

    void VisitNfa(const std::function<void(NFANode*)>& processor) const;
    void VisitNfaGroup(const NfaGroup& group, const std::function<void(NFANode*)>& processor) const;
    DfaState* CreateDfaState(Dfa& dfa, NfaGroup group) const;
};

}  // namespace cc

#endif  //CC_DFA_BUILDER_H
