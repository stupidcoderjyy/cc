//
// Created by PC on 2026/7/4.
//

#include "generate/generator.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace cc::gen {
namespace {

class GeneratorTest : public testing::Test {
protected:
    void SetUp() override { std::filesystem::remove_all("out"); }
};

TEST_F(GeneratorTest, BuildCaculator) {
    Generator gen("calculator", "cal::gen");
    // Verify no exception — ParseJson returns {} so no files are written yet
    std::cout << std::filesystem::current_path() << std::endl;
    EXPECT_NO_THROW(gen.Build("lang.txt", "out"));
}

}  // namespace
}  // namespace cc::gen
