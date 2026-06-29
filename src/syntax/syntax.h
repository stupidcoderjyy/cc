//
// Created by PC on 2026/6/30.
//

#ifndef CC_SYNTAX_H
#define CC_SYNTAX_H

#include "production.h"

namespace cc {

class Syntax {
public:
    Syntax();
    ~Syntax() = default;

    // 添加产生式 head -> body，返回产生式ID
    // 首次调用时自动插入 ROOT -> head 作为产生式0
    int AddProduction(const Symbol& head, std::initializer_list<Symbol> body);
    int AddProduction(const Symbol& head, const std::vector<Symbol>& body);

    // 若符号已存在，更新其优先级和结合性；否则添加为新符号并设置
    void SetSymbolPriority(const Symbol& sym, int priority);
    void SetSymbolAssociativity(const Symbol& sym, Associativity assoc);
    // 同时设置优先级和结合性
    void SetSymbolProperties(const Symbol& sym, int priority, Associativity assoc);

    // 为特定产生式设置优先级（覆盖符号默认值）
    void SetProductionPriority(int prodId, int priority);
    void SetProductionAssociativity(int prodId, Associativity assoc);

    // getter
    const std::vector<Production>& productions() const { return productions_; }
    const UnorderedSymbolMap<Symbol>& terminals() const { return terminals_; }
    const UnorderedSymbolMap<Symbol>& non_terminals() const { return non_terminals_; }
    const Symbol& root_symbol() const { return root_symbol_; }
    const Symbol& end_symbol() const { return end_symbol_; }

    const Symbol* FindSymbol(const std::string& name, SymbolType type) const;

private:
    std::vector<Production> productions_;
    UnorderedSymbolMap<Symbol> terminals_;
    UnorderedSymbolMap<Symbol> non_terminals_;
    Symbol root_symbol_{"ROOT", SymbolType::kNonTerminal};
    Symbol end_symbol_{"$", SymbolType::kEof};
    int next_prod_id_ = 0;

    // 内部辅助：添加符号到集合，维护终结符/非终结符分类（已存在则忽略）
    void RegisterSymbol(const Symbol& sym);
    // 插入 ROOT -> head 产生式
    void InsertRootProduction(const Symbol& head);
};

}  // namespace cc

#endif  //CC_SYNTAX_H
