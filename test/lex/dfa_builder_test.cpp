//
// Created by PC on 2026/6/29.
//
#include "lex/dfa_builder.h"

#include <gtest/gtest.h>

#include <unordered_map>

#include "compile/cc_constants.h"
#include "lex/nfa_regex_parser.h"

namespace cc {

// ---------- 辅助类：收集 DFA 输出数据 ----------
class CollectingDfaSetter : public DFASetter {
public:
    int char_class_count = -1;
    std::vector<int> char_to_class;
    int state_count = -1;
    int start_state = -1;
    std::unordered_map<int, std::pair<bool, std::string>> state_info;
    std::unordered_map<int, std::unordered_map<int, int>> transitions;

    CollectingDfaSetter() { char_to_class.resize(kMaxChars); }

    void DFASetCharClassCount(int count) override { char_class_count = count; }

    void DFASetCharToClass(int ch, int class_id) override { char_to_class[ch] = class_id; }

    void DFASetStatesCount(int count) override { state_count = count; }

    void DFASetStartState(int id) override { start_state = id; }

    void DFASetStateInfo(int stateId, bool isAccepted, const std::string& token) override {
        state_info[stateId] = {isAccepted, token};
    }

    void DFASetGoto(int start, int input, int target) override {
        transitions[start][input] = target;
    }

    // 辅助断言方法
    void ExpectStateCount(int expected) const { EXPECT_EQ(state_count, expected); }

    void ExpectStartState(int expected) const { EXPECT_EQ(start_state, expected); }

    void ExpectStateAccepted(
            int stateId, bool expected, const std::string& expectedToken = "") const {
        auto it = state_info.find(stateId);
        ASSERT_NE(it, state_info.end()) << "State " << stateId << " not found";
        EXPECT_EQ(it->second.first, expected);
        EXPECT_EQ(it->second.second, expectedToken);
    }

    void ExpectTransition(int from, int inputClass, int to) const {
        auto it1 = transitions.find(from);
        ASSERT_NE(it1, transitions.end()) << "No transitions from state " << from;
        auto it2 = it1->second.find(inputClass);
        ASSERT_NE(it2, it1->second.end())
                << "No transition for class " << inputClass << " from state " << from;
        EXPECT_EQ(it2->second, to);
    }

    void ExpectNoTransition(int from, int inputClass) const {
        auto it1 = transitions.find(from);
        if (it1 == transitions.end()) return;
        auto it2 = it1->second.find(inputClass);
        EXPECT_EQ(it2, it1->second.end())
                << "Unexpected transition for class " << inputClass << " from state " << from;
    }

    int GetClassForChar(char ch) const { return char_to_class[static_cast<unsigned char>(ch)]; }
};

// ---------- 测试用例 ----------

TEST(DfaBuilderOutputTest, OrAB) {
    NFARegexParser parser;
    parser.Register("a|b", "or_token");

    CollectingDfaSetter setter;
    DFABuilder builder(parser.root_node(), parser.node_id_to_token());
    builder.Build(&setter);

    // 验证字符类
    EXPECT_EQ(setter.char_class_count, 3);  // 'a', 'b', 其他
    int class_a = setter.GetClassForChar('a');
    int class_b = setter.GetClassForChar('b');

    setter.ExpectStateCount(2);
    setter.ExpectStartState(0);
    setter.ExpectStateAccepted(0, false);
    setter.ExpectStateAccepted(1, true, "or_token");

    setter.ExpectTransition(0, class_a, 1);
    setter.ExpectTransition(0, class_b, 1);
    for (int cid = 0; cid < setter.char_class_count; ++cid) {
        setter.ExpectNoTransition(1, cid);
    }
}

TEST(DfaBuilderOutputTest, StarA) {
    NFARegexParser parser;
    parser.Register("a*", "star_token");

    CollectingDfaSetter setter;
    DFABuilder builder(parser.root_node(), parser.node_id_to_token());
    builder.Build(&setter);

    EXPECT_EQ(setter.char_class_count, 2);
    int class_a = setter.GetClassForChar('a');

    setter.ExpectStateCount(1);
    setter.ExpectStartState(0);
    setter.ExpectStateAccepted(0, true, "star_token");
    setter.ExpectTransition(0, class_a, 0);
    for (int cid = 0; cid < setter.char_class_count; ++cid) {
        if (cid != class_a) setter.ExpectNoTransition(0, cid);
    }
}

TEST(DfaBuilderOutputTest, ConcatAB) {
    NFARegexParser parser;
    parser.Register("ab", "concat_token");

    CollectingDfaSetter setter;
    DFABuilder builder(parser.root_node(), parser.node_id_to_token());
    builder.Build(&setter);

    EXPECT_EQ(setter.char_class_count, 3);
    int class_a = setter.GetClassForChar('a');
    int class_b = setter.GetClassForChar('b');

    setter.ExpectStateCount(3);
    setter.ExpectStartState(0);
    setter.ExpectStateAccepted(0, false);
    setter.ExpectStateAccepted(1, true, "concat_token");
    setter.ExpectStateAccepted(2, false);

    setter.ExpectTransition(0, class_a, 2);
    setter.ExpectTransition(2, class_b, 1);
    for (int cid = 0; cid < setter.char_class_count; ++cid) {
        if (cid != class_a) setter.ExpectNoTransition(0, cid);
        if (cid != class_b) setter.ExpectNoTransition(2, cid);
        setter.ExpectNoTransition(1, cid);
    }
}

TEST(DfaBuilderOutputTest, StarAB) {
    NFARegexParser parser;
    parser.Register("(a|b)*", "star_ab_token");

    CollectingDfaSetter setter;
    DFABuilder builder(parser.root_node(), parser.node_id_to_token());
    builder.Build(&setter);

    EXPECT_EQ(setter.char_class_count, 3);
    int class_a = setter.GetClassForChar('a');
    int class_b = setter.GetClassForChar('b');

    setter.ExpectStateCount(1);
    setter.ExpectStartState(0);
    setter.ExpectStateAccepted(0, true, "star_ab_token");

    setter.ExpectTransition(0, class_a, 0);
    setter.ExpectTransition(0, class_b, 0);
    for (int cid = 0; cid < setter.char_class_count; ++cid) {
        if (cid != class_a && cid != class_b) setter.ExpectNoTransition(0, cid);
    }
}

TEST(DfaBuilderOutputTest, StarAOrStarB) {
    NFARegexParser parser;
    parser.Register("a*|b*", "star_or_token");

    CollectingDfaSetter setter;
    DFABuilder builder(parser.root_node(), parser.node_id_to_token());
    builder.Build(&setter);

    EXPECT_GE(setter.state_count, 2);
    EXPECT_EQ(setter.start_state, 0);

    int class_a = setter.GetClassForChar('a');
    int class_b = setter.GetClassForChar('b');

    auto& trans0 = setter.transitions[0];
    ASSERT_NE(trans0.find(class_a), trans0.end());
    ASSERT_NE(trans0.find(class_b), trans0.end());
    EXPECT_NE(trans0[class_a], trans0[class_b]);

    int target_a = trans0[class_a];
    int target_b = trans0[class_b];

    EXPECT_TRUE(setter.state_info[target_a].first);
    EXPECT_TRUE(setter.state_info[target_b].first);
    EXPECT_EQ(setter.transitions[target_a][class_a], target_a);
    EXPECT_EQ(setter.transitions[target_b][class_b], target_b);
    EXPECT_EQ(setter.state_info[target_a].second, "star_or_token");
    EXPECT_EQ(setter.state_info[target_b].second, "star_or_token");
}

}  // namespace cc