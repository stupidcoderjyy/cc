enable_testing()
include(GoogleTest)

set(TARGET cc_test)

add_executable(${TARGET}
        test_nfa.cpp
        dfa_builder_test.cpp
)

target_link_libraries(${TARGET} PRIVATE
        cc::core
        GTest::gtest_main
)

gtest_discover_tests(${TARGET})
