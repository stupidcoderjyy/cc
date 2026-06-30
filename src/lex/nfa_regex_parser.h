#ifndef CC_NFA_REGEX_PARSER_H_
#define CC_NFA_REGEX_PARSER_H_

#include <bitset>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "cc_constants.h"
#include "io/abstract_input.h"
#include "nfa.h"

namespace cc {

class NFARegexParser {
public:
    void Register(std::string regex, const std::string& token);
    void RegisterSingles(std::initializer_list<char> chs);

    NFANode* root_node() const { return root_node_; }
    std::vector<std::string> node_id_to_token() const { return node_id_to_token_; }
    const std::vector<std::unique_ptr<NFANode>>& nodes() const { return nodes_; }

private:
    std::unique_ptr<AbstractInput> input_;
    std::string regex_;
    NFANode* root_node_ = nullptr;
    std::vector<std::unique_ptr<NFANode>> nodes_;
    std::vector<std::string> node_id_to_token_;
    std::unordered_set<char> singles_;

    void SetAcceptedNode(NFA& target, const std::string& token);
    NFA Expr(bool group);
    NFA Seq();
    NFA Atom();
    NFA CheckClosure(NFA target) const;
    void Clazz(std::bitset<kMaxChars>& bitset, bool exclude);
    void MinClazzPredicate(std::bitset<kMaxChars>& bitset) const;
    void Escape(std::bitset<kMaxChars>& bitset) const;
    void MoveNode(std::unique_ptr<NFANode>& node);

    static void SetBitset(std::bitset<kMaxChars>& bitset, int begin, int end);

    [[noreturn]] void Err(const std::string& cause) const;
};

}  // namespace cc

#endif  // CC_NFA_REGEX_PARSER_H_
