set(TARGET cc_core)

add_library(${TARGET} STATIC
        test.cpp
)
add_library(cc::core ALIAS ${TARGET})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})