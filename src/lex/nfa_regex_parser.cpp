#include "nfa_regex_parser.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "io/string_input.h"

namespace cc {

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

void NFARegexParser::RegisterSingle(std::initializer_list<char> chs) {
    for (char ch : chs) {
        if (static_cast<unsigned char>(ch) > 128) {
            throw std::invalid_argument("ASCII only");
        }
        if (singles_.contains(ch)) {
            continue;
        }
        singles_.insert(ch);
        if (std::isalnum(ch)) {
            Register({ch}, "single");
        } else {
            Register(std::string("\\") + ch, "single");
        }
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
    CharPredicate p = nullptr;
    switch (b) {
        case '[':
            if (input_->Available() && input_->Forward() == '^') {
                input_->Read();
                p = Clazz(true);
            } else {
                p = Clazz(false);
            }
            break;
        case '\\':
            p = Escape();
            break;
        case '*':
        case '?':
        case '+':
            Err("invalid closure symbol");
        default:
            p = [b](int c) { return c == b; };
            break;
    }
    if (!p) {
        Err("invalid predicate");
    }
    NFA target;
    target.AndAtom(std::move(p));
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

CharPredicate NFARegexParser::Clazz(bool exclude) {
    CharPredicate result = nullptr;
    while (input_->Available()) {
        switch (input_->Read()) {
            case '[': {
                //排除模式只作用于最外层
                CharPredicate inner = Clazz(false);
                if (!inner) {
                    return nullptr;
                }
                if (!result) {
                    result = std::move(inner);
                } else {
                    result = [a = std::move(result), b = std::move(inner)](int c) {
                        return a(c) || b(c);
                    };
                }
                break;
            }
            case ']':
                if (exclude) {
                    if (!result) {
                        Err("empty character class");
                    }
                    return [p = std::move(result)](int c) { return !p(c); };
                }
                return result;
            default: {
                input_->Retract();
                CharPredicate p;
                if (input_->Forward() == '\\') {
                    input_->Read();
                    p = Escape();
                } else {
                    p = MinClazzPredicate();
                }
                if (!p) {
                    return nullptr;
                }
                if (!result) {
                    result = std::move(p);
                } else {
                    result = [a = std::move(result), p = std::move(p)](int c) {
                        return a(c) || p(c);
                    };
                }
                break;
            }
        }
    }
    return result;
}

CharPredicate NFARegexParser::MinClazzPredicate() const {
    int b = input_->Read();
    if (!input_->Available()) {
        return nullptr;
    }
    if (int b1 = input_->Read(); b1 == '-') {
        if (!input_->Available()) {
            return nullptr;
        }
        if (int b2 = input_->Read(); b2 == '[' || b2 == ']') {
            input_->Retract();
            return [b](int c) { return c == b || c == '-'; };
        } else {
            return [b, b2](int c) { return c >= b && c <= b2; };
        }
    }
    input_->Retract();
    return [b](int c) { return c == b; };
}

CharPredicate NFARegexParser::Escape() const {
    if (!input_->Available()) {
        Err("incomplete escape");
    }
    switch (int e = input_->Read()) {
        case 'D':
        case 'd':
            return [](int c) { return c >= '0' && c <= '9'; };
        case 'W':
        case 'w':
            return [](int c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                       c == '_';
            };
        case 'U':
        case 'u':
            return [](int c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
            };
        case 'H':
        case 'h':
            return [](int c) {
                return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            };
        case 'A':
        case 'a':
            return [](int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };
        case '.':
            return [](int) { return true; };
        case 'n':
            return [](int c) { return c == '\n'; };
        case 't':
            return [](int c) { return c == '\t'; };
        default:
            return [e](int c) { return c == e; };
    }
}

void NFARegexParser::MoveNode(std::unique_ptr<NFANode>& node) {
    node->set_id(static_cast<int>(nodes_.size()));
    nodes_.push_back(std::move(node));
}

void NFARegexParser::Err(const std::string& cause) const {
    std::ostringstream oss;
    oss << "failed to parse regex(" << cause << "):" << regex_;
    throw std::runtime_error(oss.str());
}

}  // namespace cc
