//
// Created by PC on 2026/6/29.
//

#ifndef CC_DFA_MINIMIZER_H
#define CC_DFA_MINIMIZER_H
#include <queue>

namespace cc {

class DFA;

class DFAMinimizer {
public:
    DFAMinimizer(DFA& dfa, int char_class_count);
    DFA Minimize();

private:
    DFA* dfa_;                                             // 原始 DFA
    int char_class_count_{};                               // 字符类数
    std::vector<std::vector<int>> blocks_;                 // 每个块的状态列表
    std::vector<int> block_of_state_;                      // 状态 -> 块 ID
    std::vector<std::vector<std::vector<int>>> incoming_;  // incoming[c][block] = 源状态列表
    std::queue<int> worklist_;

    void InitializePartition();
    void BuildIncoming();
    bool SplitBlock(int Y, const std::vector<int>& intersect, const std::vector<int>& diff);
    void UpdateIncomingAfterSplit(int old_block, int new_block1, int new_block2);
};

}  // namespace cc

#endif  //CC_DFA_MINIMIZER_H
