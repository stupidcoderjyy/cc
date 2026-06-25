enable_testing()
include(GoogleTest)

set(TARGET cc_test)

add_executable(${TARGET}
        test_gtest.cpp
)

target_link_libraries(${TARGET} PRIVATE
        cc::core
        GTest::gtest_main
)

gtest_discover_tests(${TARGET})
