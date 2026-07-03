//
// Created by PC on 2026/6/28.
//

#ifndef CC_DFA_BUILDER_H
#define CC_DFA_BUILDER_H
#include <bitset>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "cc_constants.h"
#include "dfa_setter.h"
#include "util/bitset_hash.h"

namespace cc {

using common::kMaxCharClass;
using common::kMaxChars;
using common::kMaxNfaGroups;
using common::kMaxPredicates;

typedef std::bitset<kMaxNfaGroups> NFAGroup;
typedef std::bitset<kMaxCharClass> CharClassBitMask;

template <typename V>
using NFAGroupMap = BitsetMap<V, kMaxNfaGroups>;

struct DFAState {
    using id_t = int;
    int id;
    NFAGroup nfa_group;
    bool is_accepted = false;
    std::string token;
    std::vector<id_t> class_id_to_next{};
    DFAState(int id, const NFAGroup& group, bool is_accepted, std::string token)
        : id(id), nfa_group(group), is_accepted(is_accepted), token(std::move(token)) {}
};

struct DFA {
    std::vector<std::unique_ptr<DFAState>> states;
    int start_state = -1;
    DFA() = default;
    DFA(const DFA&) = delete;
    DFA(DFA&&) = default;
    DFA& operator=(const DFA& other) = delete;
    DFA& operator=(DFA&& other) = default;
};

class NFANode;
class NFARegexParser;

//https://chat.deepseek.com/share/u04wclbb8ulsd2yibr
class DFABuilder {
public:
    explicit DFABuilder(NFARegexParser& parser);
    DFABuilder(NFANode* root_node, std::vector<std::string> nfa_node_to_token);
    void Build(const std::optional<DFASetter*>& setter = std::nullopt);
    // Visible For Testing
    void BuildCharClassMap();
    void ComputeNfaCharClassMask();
    NFAGroup EpsilonClosure(NFAGroup group);
    NFAGroup Next(int char_class, NFAGroup group);
    DFA BuildDfaFromNfa();
    DFA MinimizeDfa(DFA& dfa) const;

    // getter
    const std::vector<int>& char_to_class() const { return char_to_class_; }
    int class_count() const { return class_count_; }
    DFA& dfa() { return dfa_; }

    // setter
    void set_print_debug_info(bool enable) { print_debug_info_ = enable; }
    void set_print_conflict_info(bool enable) { print_conflict_info_ = enable; }
    void set_disable_dfa_minimize_(bool disable) { disable_dfa_minimize_ = disable; }

private:
    NFANode* root_node_;
    std::vector<int> char_to_class_;
    int class_count_{};
    // 若nfa_node_to_char_class_[node][cid]，则NFA结点node接受cid字符类中的字符
    std::vector<CharClassBitMask> nfa_node_to_char_class_;
    std::vector<char> class_representative_char_;
    std::vector<std::string> nfa_node_to_token_;
    std::unordered_map<NFAGroup, NFAGroup> epsilon_cache_;
    std::vector<int> shared_stack_;
    // 不持有结点所有权
    std::vector<NFANode*> nfa_nodes_;
    DFA dfa_;
    bool print_debug_info_ = false;
    bool print_conflict_info_ = false;
    bool disable_dfa_minimize_ = false;

    void VisitNfa(const std::function<void(NFANode*)>& processor) const;
    void VisitNfaGroup(const NFAGroup& group, const std::function<void(NFANode*)>& processor) const;
    DFAState* CreateDfaState(DFA& dfa, NFAGroup group) const;
    void OutputData(DFASetter& setter) const;

    void PrintNFA() const;
    void PrintNFAToClass() const;
    void PrintCharToClass() const;
    void PrintDFANodes() const;
    static void PrintState(int state, bool accept);
    static void PrintGoto(int src_state, int target, bool target_accept);
    static void WarnTokenMatchConflict(DFAState& state, std::vector<std::string>& tokens);
};

}  // namespace cc

#endif  //CC_DFA_BUILDER_H
