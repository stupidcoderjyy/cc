//
// Created by PC on 2026/6/29.
//

#include "dfa_minimizer.h"

#include "dfa_builder.h"

namespace cc {

DfaMinimizer::DfaMinimizer(Dfa& dfa, int char_class_count)
    : dfa_(&dfa), char_class_count_(char_class_count) {}

Dfa DfaMinimizer::Minimize() {
    if (dfa_->states.empty()) {
        return {};
    }
    InitializePartition();
    BuildIncoming();

    while (!worklist_.empty()) {
        int b1 = worklist_.front();
        worklist_.pop();

        bool split_occurred = false;
        int cur_block_count = static_cast<int>(blocks_.size());

        for (int c = 0; c < char_class_count_ && !split_occurred; ++c) {
            const auto& sources = incoming_[c][b1];
            if (sources.empty()) continue;

            // 标记哪些状态能通过字符 c 到达块 B
            std::vector in_sources(dfa_->states.size(), false);
            for (int s : sources) {
                in_sources[s] = true;
            }
            // 遍历当前所有块（只检查分裂前存在的块）
            for (int b2 = 0; b2 < cur_block_count && !split_occurred; ++b2) {
                const auto& block_states = blocks_[b2];
                std::vector<int> intersect, diff;
                for (int s : block_states) {
                    if (in_sources[s]) {
                        intersect.push_back(s);
                    } else {
                        diff.push_back(s);
                    }
                }

                if (!intersect.empty() && !diff.empty()) {
                    // 执行分裂
                    split_occurred = SplitBlock(b2, intersect, diff);
                }
            }
        }
    }

    // 构建最小化 DFA
    Dfa min_dfa;
    min_dfa.states.clear();  // 移除 Dfa 默认构造函数添加的空状态

    int new_n = static_cast<int>(blocks_.size());
    min_dfa.states.reserve(new_n);

    // 创建新状态
    for (int b = 0; b < new_n; ++b) {
        int rep = blocks_[b][0];
        const auto& old_state = dfa_->states[rep];
        auto new_state =
            std::make_unique<DfaState>(b, NfaGroup(), old_state->is_accepted, old_state->token);
        new_state->class_id_to_next.assign(char_class_count_, -1);
        min_dfa.states.push_back(std::move(new_state));
    }

    // 填充转移
    for (int b = 0; b < new_n; ++b) {
        int rep = blocks_[b][0];
        const auto& trans = dfa_->states[rep]->class_id_to_next;
        for (int c = 0; c < char_class_count_; ++c) {
            if (int t = trans[c]; t != -1) {
                int target_block = block_of_state_[t];
                min_dfa.states[b]->class_id_to_next[c] = target_block;
            }
        }
    }

    min_dfa.start_state = block_of_state_[dfa_->start_state];
    return min_dfa;
}

// ---------- 子函数实现 ----------

void DfaMinimizer::InitializePartition() {
    const auto& states = dfa_->states;
    int n = static_cast<int>(states.size());

    // 按 token 分组接受状态
    std::unordered_map<std::string, std::vector<int>> token_groups;
    std::vector<int> nt_states;

    for (int i = 0; i < n; ++i) {
        if (const auto& st = states[i]; st->is_accepted) {
            token_groups[st->token].push_back(i);
        } else {
            nt_states.push_back(i);
        }
    }

    blocks_.clear();
    block_of_state_.assign(n, -1);
    while (!worklist_.empty())
        worklist_.pop();

    auto add_block = [this](const std::vector<int>& block_states) {
        int bid = static_cast<int>(blocks_.size());
        blocks_.push_back(block_states);
        for (int s : block_states)
            block_of_state_[s] = bid;
        worklist_.push(bid);
    };

    if (!nt_states.empty()) {
        add_block(nt_states);
    }
    for (auto& t_states : token_groups | std::views::values) {
        add_block(t_states);
    }
}

void DfaMinimizer::BuildIncoming() {
    int block_count = static_cast<int>(blocks_.size());
    incoming_.assign(char_class_count_, std::vector<std::vector<int>>(block_count));

    for (int s = 0; s < static_cast<int>(dfa_->states.size()); ++s) {
        const auto& trans = dfa_->states[s]->class_id_to_next;
        for (int c = 0; c < char_class_count_; ++c) {
            if (int t = trans[c]; t != -1) {
                int target_block = block_of_state_[t];
                incoming_[c][target_block].push_back(s);
            }
        }
    }
}

bool DfaMinimizer::SplitBlock(int Y, const std::vector<int>& intersect,
                              const std::vector<int>& diff) {
    // 两者均非空
    if (intersect.empty() || diff.empty()) return false;

    int old_block = Y;
    // 保留 intersect 为旧块，新建 diff
    blocks_[old_block] = intersect;
    int new_block = static_cast<int>(blocks_.size());
    blocks_.push_back(diff);

    // 更新 block_of_state_
    for (int s : diff)
        block_of_state_[s] = new_block;

    // 扩展 incoming_ 维度（新增一个块）
    for (int c = 0; c < char_class_count_; ++c) {
        incoming_[c].resize(blocks_.size());
    }

    // 增量更新反向转移表
    UpdateIncomingAfterSplit(old_block, old_block, new_block);

    // 将两个块加入工作队列
    worklist_.push(old_block);
    worklist_.push(new_block);
    return true;
}

void DfaMinimizer::UpdateIncomingAfterSplit(int old_block, int new_block1, int new_block2) {
    // new_block1 和 new_block2 分别为分裂后的两个块 ID（这里 old_block 和 new_block）
    for (int c = 0; c < char_class_count_; ++c) {
        auto& sources = incoming_[c][old_block];
        if (sources.empty()) continue;

        // 清空新块可能残留的数据（安全起见）
        incoming_[c][new_block1].clear();
        incoming_[c][new_block2].clear();

        // 重新分配每个源状态
        for (int s : sources) {
            if (int t = dfa_->states[s]->class_id_to_next[c]; t != -1) {
                int target_block = block_of_state_[t];
                // target_block 必定是 new_block1 或 new_block2
                incoming_[c][target_block].push_back(s);
            }
        }
        sources.clear();  // 清空旧块列表
    }
}

}  // namespace cc
