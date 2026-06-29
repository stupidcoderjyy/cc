//
// Created by PC on 2026/6/29.
//

#ifndef CC_DFA_SETTER_H
#define CC_DFA_SETTER_H
#include <string>
#include <vector>

namespace cc {
/**
 * 抽象接口：用于输出 DFA 构建结果。
 * 派生类可实现为文件写入、代码生成、序列化等。
 */
class DfaSetter {
public:
    virtual ~DfaSetter() = default;

    // 设置字符类总数（可选，但可用于验证）
    virtual void SetCharClassCount(int count) = 0;

    // 设置字符到类的映射，向量长度 = 字符集大小（如 128 或 256）
    // 索引为字符值（如 ASCII 码），值为对应的类 ID (0 ~ count-1)
    virtual void SetCharToClass(const std::vector<int>& char_to_class) = 0;

    // 设置 DFA 状态总数（在添加具体状态前调用）
    virtual void SetDfaStatesCount(int count) = 0;

    // 设置起始状态 ID
    virtual void SetStartState(int id) = 0;

    // 设置某个状态是否为接受状态及其对应的 token（若接受）
    // 若为非接受状态，token 可为空字符串
    virtual void SetStateInfo(int stateId, bool isAccepted, const std::string& token) = 0;

    // 设置一条转移：从 start 状态，输入字符类 input，转移到 target 状态
    // input 为类索引（0 ~ class_count-1），target 为目标状态 ID（-1 表示无转移，可不调用）
    virtual void SetTransition(int start, int input, int target) = 0;

    // 可选：完成所有设置后调用（如写入文件尾）
    virtual void Finish() {}
};

}  // namespace cc

#endif  //CC_DFA_SETTER_H
