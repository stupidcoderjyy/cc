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
class LanguageSetter {
public:
    virtual ~LanguageSetter() = default;

    // 输出产生式列表
    virtual void SetProductions(const std::vector<Production>& prods) = 0;

    // 输出符号表（含优先级和结合性）
    virtual void SetTerminals(const std::vector<Symbol>& terms) = 0;
    virtual void SetNonTerminals(const std::vector<Symbol>& non_terms) = 0;

    // 输出 LALR 状态（每状态的项集和前瞻符）
    virtual void SetLALRState(int stateId, const std::set<Item>& items) = 0;
    virtual void SetStateLookaheads(int stateId, const std::map<Item, std::set<Symbol, std::less<>>>& la) = 0;

    // 输出分析表
    virtual void SetAction(int stateId, int symbolId, int type, int target) = 0;
    virtual void SetGoto(int stateId, int symbolId, int target) = 0;

    // 设置起始状态
    virtual void SetStartState(int stateId) = 0;

    // 全部数据设置完毕后调用
    virtual void Finish() {}
};

}  // namespace cc

#endif  //CC_LANGUAGE_SETTER_H
