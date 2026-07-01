//
// Created by PC on 2026/6/30.
//

#ifndef CC_LIMITS_H
#define CC_LIMITS_H

#include <climits>

namespace common {

static constexpr int kMaxChars = CHAR_MAX;  // 可输入的最大字符
static constexpr int kMaxPredicates = 128;  // 最大字符集数
static constexpr int kMaxNfaGroups = 128;   // 最大NFA状态组数量
static constexpr int kMaxCharClass = 64;    // 最大字符类数量

}  // namespace common

#endif  //CC_LIMITS_H
