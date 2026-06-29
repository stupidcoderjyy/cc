//
// Created by PC on 2026/6/30.
//

#include "lalr_conflict_handler.h"

namespace cc {

ConflictAction DefaultConflictHandler::HandleShiftReduce(int /*stateId*/, int /*symbolId*/,
                                                         int /*shiftTarget*/, int /*reduceProdId*/,
                                                         int reducePriority,
                                                         Associativity reduceAssoc,
                                                         int lookaheadPriority,
                                                         Associativity lookaheadAssoc) {
    if (reducePriority > lookaheadPriority) return ConflictAction::kReduce;
    if (lookaheadPriority > reducePriority) return ConflictAction::kShift;

    // 同优先级，按结合性
    if (reduceAssoc == Associativity::kLeft) return ConflictAction::kReduce;
    if (reduceAssoc == Associativity::kRight) return ConflictAction::kShift;
    return ConflictAction::kAbort;
}

int DefaultConflictHandler::HandleReduceReduce(int /*stateId*/, int /*symbolId*/,
                                               const std::vector<int>& /*reduceProdIds*/) {
    return -1;  // 默认无法解决
}

}  // namespace cc
