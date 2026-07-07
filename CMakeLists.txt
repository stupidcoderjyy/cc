cmake_minimum_required(VERSION 4.0)

project(stupidcoder.cc LANGUAGES CXX)
set(PROJECT_VERSION "1.0.0")   # 设置版本号

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

list(PREPEND CMAKE_PREFIX_PATH "C:/msys64/mingw64")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

find_package(inja REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
        stdc++exp
        stupidcoder::cc_core
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
            "${PROJECT_SOURCE_DIR}/cc/*.cpp"
            "${PROJECT_SOURCE_DIR}/cc/*.h"
            "${PROJECT_SOURCE_DIR}/common/*.cpp"
            "${PROJECT_SOURCE_DIR}/common/*.h"
            "${PROJECT_SOURCE_DIR}/test/*.cpp"
            "${PROJECT_SOURCE_DIR}/examples/*.h"
            "${PROJECT_SOURCE_DIR}/examples/*.cpp"
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
add_subdirectory(cc)
add_subdirectory(common)
add_subdirectory(test)
add_subdirectory(examples)

# 2. 安装可执行文件到 bin/ 子目录
install(TARGETS ${PROJECT_NAME} cc_core common
        EXPORT stupidcoderTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib      # Linux .so / macOS .dylib
        ARCHIVE DESTINATION lib
)

install(EXPORT stupidcoderTargets
        FILE stupidcoderTargets.cmake
        NAMESPACE stupidcoder::   # 可选：为目标添加命名空间前缀
        DESTINATION lib/cmake/stupidcoder
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

# 生成和安装 Config 文件
include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/cmake/stupidcoderConfig.cmake.in
        ${CMAKE_BINARY_DIR}/stupidcoderConfig.cmake
        INSTALL_DESTINATION lib/cmake/stupidcoder
)
write_basic_package_version_file(
        ${CMAKE_BINARY_DIR}/stupidcoderConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
install(FILES
        ${CMAKE_BINARY_DIR}/stupidcoderConfig.cmake
        ${CMAKE_BINARY_DIR}/stupidcoderConfigVersion.cmake
        DESTINATION lib/cmake/stupidcoder
)