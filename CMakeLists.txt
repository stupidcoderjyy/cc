cmake_minimum_required(VERSION 4.0)

project(cc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEAS "-O2 -DNDEBUG")

set(LIB_PREFIX_PATH "C:/msys64/mingw64")
list(PREPEND CMAKE_PREFIX_PATH ${LIB_PREFIX_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
        stdc++exp
        jyy::cc_core
)

# 根据构建类型决定是否生成共享库
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_SHARED_LIBS ON)
    # Windows 下自动导出所有符号（CMake 3.4+）
    if(WIN32)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
else()
    set(BUILD_SHARED_LIBS OFF)
endif()

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
            "${PROJECT_SOURCE_DIR}/test/*.cpp"
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
add_subdirectory(templates)
add_subdirectory(cc)
add_subdirectory(common)
add_subdirectory(test)
add_subdirectory(examples)

# 模板
install(FILES
        ${CMAKE_SOURCE_DIR}/templates/parser_data.cpp.txt
        ${CMAKE_SOURCE_DIR}/templates/parser_data.h.txt
        DESTINATION bin/templates
)

# 2. 安装可执行文件到 bin/ 子目录
install(TARGETS ${PROJECT_NAME} cc_core cc_common
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib      # Linux .so / macOS .dylib
        ARCHIVE DESTINATION lib
)
# 头文件
install(DIRECTORY ${CMAKE_SOURCE_DIR}/cc
        DESTINATION include/stupidcoder
        FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/common
        DESTINATION include/stupidcoder
        FILES_MATCHING PATTERN "*.h"
)