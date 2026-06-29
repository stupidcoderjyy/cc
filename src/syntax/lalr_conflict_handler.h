//
// Created by PC on 2026/6/30.
//

#ifndef CC_LALR_ERROR_HANDLER_H
#define CC_LALR_ERROR_HANDLER_H

#include <vector>

#include "production.h"

namespace cc {

// 冲突解决策略
enum class ConflictAction { kShift, kReduce, kAbort };

// 分析表动作
enum class ActionType { kError, kShift, kReduce, kAccept };

struct Action {
    ActionType type = ActionType::kError;
    int target = -1;  // SHIFT: target state, REDUCE: production ID
};

// 冲突处理抽象接口
class LALRConflictHandler {
public:
    virtual ~LALRConflictHandler() = default;

    // 移入-规约冲突
    // reducePriority/reduceAssoc: 规约产生式的优先级/结合性
    // lookaheadPriority/lookaheadAssoc: 向前看终结符的优先级/结合性
    virtual ConflictAction HandleShiftReduce(int stateId, int symbolId, int shiftTarget,
                                             int reduceProdId, int reducePriority,
                                             Associativity reduceAssoc, int lookaheadPriority,
                                             Associativity lookaheadAssoc) = 0;

    // 规约-规约冲突：返回选中的产生式ID，或 -1 表示放弃
    virtual int HandleReduceReduce(int stateId, int symbolId,
                                   const std::vector<int>& reduceProdIds) = 0;
};

// 默认实现：基于优先级和结合性自动解决冲突
class DefaultConflictHandler : public LALRConflictHandler {
public:
    ConflictAction HandleShiftReduce(int stateId, int symbolId, int shiftTarget, int reduceProdId,
                                     int reducePriority, Associativity reduceAssoc,
                                     int lookaheadPriority, Associativity lookaheadAssoc) override;

    int HandleReduceReduce(int stateId, int symbolId,
                           const std::vector<int>& reduceProdIds) override;
};

}  // namespace cc

#endif  //CC_LALR_ERROR_HANDLER_H
