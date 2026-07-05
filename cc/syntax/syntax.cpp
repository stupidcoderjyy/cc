//
// Created by PC on 2026/6/30.
//
#include "syntax.h"

#include <algorithm>
#include <stdexcept>

namespace cc {

Syntax::Syntax() {
    RegisterSymbol(root_symbol_);
}

int Syntax::AddProduction(const Production& temp) {
    Symbol head(temp.head.name, SymbolType::kNonTerminal, temp.priority, temp.assoc);
    return AddProduction(head, temp.body);
}

int Syntax::AddProduction(const Symbol& head, std::initializer_list<Symbol> body) {
    return AddProduction(head, std::vector(body.begin(), body.end()));
}

int Syntax::AddProduction(const Symbol& head, const std::vector<Symbol>& body) {
    // 将左部加入非终结符集合
    RegisterSymbol(head);
    // 将右部所有符号加入对应的集合（终结符/非终结符）
    for (const auto& sym : body) {
        RegisterSymbol(sym);
    }

    // 首次注册产生式时，自动插入 ROOT -> head 作为产生式0
    if (productions_.empty()) {
        InsertRootProduction(head);
    }

    int prod_id = next_prod_id_++;
    productions_.emplace_back(prod_id, head, body, head.priority, head.assoc);
    return prod_id;
}

void Syntax::InsertRootProduction(const Symbol& head) {
    int prod_id = next_prod_id_++;
    productions_.emplace_back(prod_id, root_symbol_, std::vector{head}, 0, Associativity::kLeft);
}

void Syntax::SetSymbolPriority(const Symbol& sym, int priority) {
    auto& map = sym.type == SymbolType::kTerminal ? terminals_ : non_terminals_;
    if (auto it = map.find(sym); it != map.end()) {
        it->second.priority = priority;
    } else {
        Symbol new_sym = sym;
        new_sym.priority = priority;
        map.emplace(new_sym, new_sym);
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
    auto it = std::ranges::find_if(
            productions_, [prodId](const Production& p) { return p.id == prodId; });
    if (it != productions_.end()) {
        it->priority = priority;
    }
}

void Syntax::SetProductionAssociativity(int prodId, Associativity assoc) {
    auto it = std::ranges::find_if(
            productions_, [prodId](const Production& p) { return p.id == prodId; });
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
