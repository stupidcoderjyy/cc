//
// Created by PC on 2026/6/28.
//

#ifndef CC_DFA_BUILDER_H
#define CC_DFA_BUILDER_H
#include <bitset>
#include <climits>
#include <functional>
#include <memory>
#include <unordered_set>

namespace cc {

class NFANode;
class NFARegexParser;

//https://chat.deepseek.com/share/u04wclbb8ulsd2yibr
class DfaBuilder {
public:
    static constexpr int kMaxChars = CHAR_MAX;  // 可输入的最大字符
    static constexpr int kMaxPredicates = 128;  // 最大字符集数
    static constexpr int kMaxNfaGroups = 128;   // 最大NFA状态组数量
    typedef std::bitset<kMaxNfaGroups> NfaGroup;

    explicit DfaBuilder(NFANode* root_node);
    void Build();
    void BuildCharClassMap();
    NfaGroup EpsilonClosure(NfaGroup group);

    const std::vector<int>& char_to_class() const { return char_to_class_; }
    int class_count() const { return class_count_; }

private:
    NFANode* root_node_;
    std::vector<int> char_to_class_;
    int class_count_{};
    std::unordered_map<NfaGroup, NfaGroup> epsilon_cache_;
    // 不持有结点所有权
    std::vector<NFANode*> id_to_node_;

    void VisitNfa(const std::function<void(NFANode*)>& processor) const;
};

}  // namespace cc

#endif  //CC_DFA_BUILDER_H
