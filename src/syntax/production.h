//
// Created by PC on 2026/6/30.
//

#ifndef CC_SYMBOL_H
#define CC_SYMBOL_H
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace cc {

enum class SymbolType { kTerminal, kNonTerminal, kEof };

enum class Associativity { kLeft, kRight, kNonAssoc };

struct Symbol {
    std::string name;
    SymbolType type;
    int priority = 0;  // 优先级（数值越大优先级越高）
    Associativity assoc = Associativity::kLeft;

    Symbol(std::string name, SymbolType type, int priority, Associativity assoc)
        : name(std::move(name)), type(type), priority(priority), assoc(assoc) {}
    Symbol(std::string name, SymbolType type) : name(std::move(name)), type(type) {}
    Symbol() : type() {}

    bool operator==(const Symbol& other) const { return name == other.name && type == other.type; }
    bool operator<(const Symbol& other) const {
        if (type != other.type) return type < other.type;
        return name < other.name;
    }
};

struct SymbolHash {
    std::size_t operator()(const Symbol& s) const {
        return std::hash<std::string>()(s.name) ^ static_cast<int>(s.type) << 1;
    }
};

template <typename V>
using UnorderedSymbolMap = std::unordered_map<Symbol, V, SymbolHash>;

template <typename V>
using SymbolMap = std::map<Symbol, V, std::less<>>;

using SymbolSet = std::set<Symbol, std::less<>>;

struct Production {
    int id{};                                    // 唯一编号
    Symbol lhs;                                  // 左部非终结符
    std::vector<Symbol> rhs;                     // 右部符号序列（可为空，表示 ε）
    int priority = 0;                            // 产生式自身的优先级（用于冲突解决）
    Associativity assoc = Associativity::kLeft;  // 产生式结合性

    Production(int id, Symbol lhs, std::vector<Symbol> rhs, int prio, Associativity a)
        : id(id), lhs(std::move(lhs)), rhs(std::move(rhs)), priority(prio), assoc(a) {}
};

}  // namespace cc

#endif  //CC_SYMBOL_H
