//
// Created by PC on 2026/6/28.
//

#ifndef CC_DFA_BUILDER_H
#define CC_DFA_BUILDER_H
#include <climits>
#include <functional>

namespace cc {

class NFANode;

//https://chat.deepseek.com/share/u04wclbb8ulsd2yibr
class DfaBuilder {
public:
    explicit DfaBuilder(NFANode* root_node);
    void Build();
    void BuildCharClassMap();

    static constexpr int kMaxChars = CHAR_MAX;  // 可输入的最大字符
    static constexpr int kMaxPredicates = 128;  // 最大字符集数
    const std::vector<int>& char_to_class() const { return char_to_class_; }
    int class_count() const { return class_count_; }

private:
    NFANode* root_node_;
    std::vector<int> char_to_class_;
    int class_count_{};

    void VisitNfa(const std::function<void(NFANode*)>& processor) const;
};

}  // namespace cc

#endif  //CC_DFA_BUILDER_H
