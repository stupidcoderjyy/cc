cmake_minimum_required(VERSION 4.0)

project(cc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEAS "-O2 -DNDEBUG")

set(LIB_PREFIX_PATH "C:/msys64/mingw64")
list(PREPEND CMAKE_PREFIX_PATH ${LIB_PREFIX_PATH})

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
        stdc++exp
        cc::core
)

# 代码格式化
find_program(CLANG_FORMAT
        NAMES clang-format
        PATHS ${CMAKE_PREFIX_PATH}/bin
        NO_DEFAULT_PATH  # 仅搜索指定路径，不搜索系统默认路径
)
if(NOT CLANG_FORMAT)
    find_program(CLANG_FORMAT NAMES clang-format)
endif()

if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
            "${PROJECT_SOURCE_DIR}/src/*.cpp"
            "${PROJECT_SOURCE_DIR}/src/*.h"
    )

    add_custom_target(format
            COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_SOURCE_FILES}
            COMMENT "Running clang-format on all source files."
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    add_dependencies(${PROJECT_NAME} format)

    message(STATUS "clang-format found at ${CLANG_FORMAT}")
else()
    message(WARNING "clang-format not found. The 'format' target will not be available.")
endif()

add_subdirectory(thirdparty)
add_subdirectory(src)
add_subdirectory(test)