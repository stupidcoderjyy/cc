//
// Created by PC on 2026/6/30.
//

#ifndef CC_LANGUAGE_SETTER_H
#define CC_LANGUAGE_SETTER_H

#include <vector>

namespace cc {

struct Symbol;
struct Production;
struct Item;
struct LR0State;

// LALR 构造结果输出接口
struct LanguageSetter {
    virtual ~LanguageSetter() = default;

    virtual void LALRSetStatesCount(int count) = 0;
    // 输出产生式列表
    virtual void LALRSetProductions(const std::vector<Production>& prods) = 0;

    // 输出符号表（含优先级和结合性）
    virtual void LALRSetTerminals(const std::vector<Symbol>& symbols) = 0;
    virtual void LALRSetNonTerminals(const std::vector<Symbol>& symbols) = 0;

    // 输出分析表
    virtual void LALRSetAction(int stateId, int symbolId, int type, int target) = 0;
    virtual void LALRSetGoto(int stateId, int symbolId, int target) = 0;

    // 全部数据设置完毕后调用
    virtual void LALRFinish() {}
};

}  // namespace cc

#endif  //CC_LANGUAGE_SETTER_H
