//
// Created by PC on 2026/6/25.
//

#include <gtest/gtest.h>
#include "test.h"

// 一个简单的测试用例
TEST(HelloTest, BasicAssertions) {
    // 期望两个整数相等
    EXPECT_EQ(TestNum(), 10);
    // 期望条件为真
    EXPECT_TRUE(true);
}