//
// Created by PC on 2026/6/30.
//
#include "syntax.h"

#include <algorithm>
#include <stdexcept>

namespace cc {

int Syntax::AddProduction(const Symbol& lhs, std::initializer_list<Symbol> rhs) {
    return AddProduction(lhs, std::vector(rhs.begin(), rhs.end()));
}

int Syntax::AddProduction(const Symbol& lhs, const std::vector<Symbol>& rhs) {
    // 将左部加入非终结符集合
    RegisterSymbol(lhs);
    // 将右部所有符号加入对应的集合（终结符/非终结符）
    for (const auto& sym : rhs) {
        RegisterSymbol(sym);
    }

    // 创建产生式对象，默认优先级和结合性从左部符号继承
    int prod_id = next_prod_id_++;
    Production prod(prod_id, lhs, rhs, lhs.priority, lhs.assoc);
    productions_.push_back(prod);
    return prod_id;
}

void Syntax::SetStartSymbol(const Symbol& start) {
    if (start.type != SymbolType::kNonTerminal) {
        throw std::runtime_error("StartSymbol must be a non-terminal");
    }
    start_symbol_ = start;
    // 确保起始符号已被记录（若尚未添加则加入非终结符）
    RegisterSymbol(start);
}

void Syntax::SetSymbolPriority(const Symbol& sym, int priority) {
    auto& map = sym.type == SymbolType::kTerminal ? terminals_ : non_terminals_;
    if (auto it = map.find(sym); it != map.end()) {
        it->second.priority = priority;  // 更新已有符号
    } else {
        Symbol new_sym = sym;
        new_sym.priority = priority;
        map.emplace(new_sym, new_sym);  // 新增符号
    }
}

void Syntax::SetSymbolAssociativity(const Symbol& sym, Associativity assoc) {
    auto& map = sym.type == SymbolType::kTerminal ? terminals_ : non_terminals_;
    if (auto it = map.find(sym); it != map.end()) {
        it->second.assoc = assoc;
    } else {
        Symbol new_sym = sym;
        new_sym.assoc = assoc;
        map.emplace(new_sym, new_sym);
    }
}

void Syntax::SetSymbolProperties(const Symbol& sym, int priority, Associativity assoc) {
    auto& map = sym.type == SymbolType::kTerminal ? terminals_ : non_terminals_;
    if (auto it = map.find(sym); it != map.end()) {
        it->second.priority = priority;
        it->second.assoc = assoc;
    } else {
        Symbol new_sym = sym;
        new_sym.priority = priority;
        new_sym.assoc = assoc;
        map.emplace(new_sym, new_sym);
    }
}

void Syntax::SetProductionPriority(int prodId, int priority) {
    auto it = std::ranges::find_if(productions_,
                                   [prodId](const Production& p) { return p.id == prodId; });
    if (it != productions_.end()) {
        it->priority = priority;
    }
}

void Syntax::SetProductionAssociativity(int prodId, Associativity assoc) {
    auto it = std::ranges::find_if(productions_,
                                   [prodId](const Production& p) { return p.id == prodId; });
    if (it != productions_.end()) {
        it->assoc = assoc;
    }
}

const Symbol* Syntax::FindSymbol(const std::string& name, SymbolType type) const {
    Symbol key;
    key.name = name;
    key.type = type;
    if (type == SymbolType::kTerminal) {
        auto it = terminals_.find(key);
        if (it != terminals_.end()) return &it->second;
    } else if (type == SymbolType::kNonTerminal) {
        auto it = non_terminals_.find(key);
        if (it != non_terminals_.end()) return &it->second;
    }
    return nullptr;
}

void Syntax::RegisterSymbol(const Symbol& sym) {
    // 忽略 EOF 符号（它单独存储，不放入终结符/非终结符集合）
    if (sym.type == SymbolType::kEof) return;

    auto& map = sym.type == SymbolType::kTerminal ? terminals_ : non_terminals_;
    // 若不存在则插入，已存在则忽略（保留原有属性）
    map.emplace(sym, sym);
}

}  // namespace cc