#include "nfa_regex_parser.h"

#include <stdexcept>
#include <utility>

#include "cc_constants.h"
#include "io/string_input.h"

namespace cc {

using common::StringInput;

void NFARegexParser::Register(std::string regex, const std::string& token) {
    if (regex.empty()) {
        return;
    }
    input_ = std::make_unique<StringInput>(regex);
    regex_ = std::move(regex);
    auto regex_nfa = Expr(false);
    // 将NFA结点所有权从NFA对象转到Parser，设置结点id
    nodes_.reserve(nodes_.size() + regex_nfa.nodes().size());
    for (auto& node : regex_nfa.nodes()) {
        MoveNode(node);
    }
    SetAcceptedNode(regex_nfa, token);
    if (!root_node_) {
        root_node_ = regex_nfa.start();
    } else {
        auto new_start = std::make_unique<NFANode>();
        new_start->AddEpsilonEdge(regex_nfa.start(), root_node_);
        root_node_ = new_start.get();
        MoveNode(new_start);
    }
}

void NFARegexParser::SetAcceptedNode(NFA& target, const std::string& token) {
    target.end()->set_accepted(true);
    int end_id = target.end()->id();
    if (static_cast<size_t>(end_id) >= node_id_to_token_.size()) {
        node_id_to_token_.resize(end_id + 1);
    }
    node_id_to_token_[end_id] = token;
}

NFA NFARegexParser::Expr(bool group) {
    NFA result;
    while (input_->Available()) {
        switch (input_->Read()) {
            case ')':
                if (!group) {
                    Err("unclosed group");
                }
                return result;
            case '|':
                if (!input_->Available()) {
                    Err("missing expr");
                }
                input_->Read();
                result.Or(Seq());
                break;
            default:
                result.And(Seq());
                break;
        }
    }
    if (group) {
        Err("unclosed group");
    }
    return result;
}

NFA NFARegexParser::Seq() {
    input_->Retract();
    NFA result;
    while (input_->Available()) {
        switch (input_->Read()) {
            case '(':
                result.And(CheckClosure(Expr(true)));
                break;
            case '|':
            case ')':
                input_->Retract();
                return result;
            default:
                result.And(Atom());
                break;
        }
    }
    return result;
}

NFA NFARegexParser::Atom() {
    input_->Retract();
    int b = input_->Read();
    std::bitset<kMaxChars> bitset;
    switch (b) {
        case '[':
            if (input_->Available() && input_->Forward() == '^') {
                input_->Read();
                Clazz(bitset, true);
            } else {
                Clazz(bitset, false);
            }
            break;
        case '\\':
            Escape(bitset);
            break;
        case '*':
        case '?':
        case '+':
            Err("invalid closure symbol");
        case '.':
            bitset.set();
            break;
        default:
            bitset.set(b);
            break;
    }
    NFA target;
    target.AndAtom([bitset](int ch) { return bitset.test(ch); });
    return CheckClosure(std::move(target));
}

NFA NFARegexParser::CheckClosure(NFA target) const {
    if (input_->Available()) {
        switch (input_->Read()) {
            case '*':
                target.Star();
                break;
            case '+':
                target.Plus();
                break;
            case '?':
                target.Quest();
                break;
            default:
                input_->Retract();
                break;
        }
    }
    return target;
}

void NFARegexParser::Clazz(std::bitset<kMaxChars>& bitset, bool exclude) {
    while (input_->Available()) {
        switch (input_->Read()) {
            case '[': {
                //排除模式只作用于最外层
                Clazz(bitset, false);
                break;
            }
            case ']':
                if (!bitset.any()) {
                    Err("empty character class");
                }
                if (exclude) {
                    bitset.flip();
                }
                return;
            default: {
                input_->Retract();
                if (input_->Forward() == '\\') {
                    input_->Read();
                    Escape(bitset);
                } else {
                    MinClazzPredicate(bitset);
                }
                break;
            }
        }
    }
}

void NFARegexParser::MinClazzPredicate(std::bitset<kMaxChars>& bitset) const {
    int b = input_->Read();
    if (!input_->Available()) {
        Err("incomplete class");
    }
    if (int b1 = input_->Read(); b1 == '-') {
        if (!input_->Available()) {
            Err("incomplete class");
        }
        if (int b2 = input_->Read(); b2 == '[' || b2 == ']') {
            input_->Retract();
            bitset.set(b);
            bitset.set('-');
        } else {
            SetBitset(bitset, b, b2);
        }
    }
    input_->Retract();
    bitset.set(b);
}

void NFARegexParser::Escape(std::bitset<kMaxChars>& bitset) const {
    if (!input_->Available()) {
        Err("incomplete escape");
    }
    switch (int e = input_->Read()) {
        case 'D':
        case 'd':
            SetBitset(bitset, '0', '9');
            break;
        case 'W':
        case 'w':
            SetBitset(bitset, 'a', 'z');
            SetBitset(bitset, 'A', 'Z');
            SetBitset(bitset, '0', '9');
            bitset.set('_');
            break;
        case 'U':
        case 'u':
            SetBitset(bitset, 'a', 'z');
            SetBitset(bitset, 'A', 'Z');
            SetBitset(bitset, '0', '9');
            break;
        case 'H':
        case 'h':
            SetBitset(bitset, 'a', 'f');
            SetBitset(bitset, 'A', 'F');
            SetBitset(bitset, '0', '9');
            break;
        case 'A':
        case 'a':
            SetBitset(bitset, 'a', 'z');
            SetBitset(bitset, 'A', 'Z');
            break;
        case 'n':
            bitset.set('\n');
            break;
        case 't':
            bitset.set('\t');
            break;
        default:
            bitset.set(e);
            break;
    }
}

void NFARegexParser::MoveNode(std::unique_ptr<NFANode>& node) {
    node->set_id(static_cast<int>(nodes_.size()));
    nodes_.push_back(std::move(node));
}

void NFARegexParser::SetBitset(std::bitset<kMaxChars>& bitset, int begin, int end) {
    if (begin < 0) begin = 0;
    if (end > kMaxChars) end = kMaxChars;
    if (begin >= end) return;
    while (begin < end) {
        bitset.set(begin++);
    }
}

void NFARegexParser::Err(const std::string& cause) const {
    std::ostringstream oss;
    oss << "failed to parse regex(" << cause << "):" << regex_;
    throw std::runtime_error(oss.str());
}

}  // namespace cc
