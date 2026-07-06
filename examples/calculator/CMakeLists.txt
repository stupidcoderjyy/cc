set(TARGET example_calculator)

# 生成文件路径
set(GENERATED_FILES
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/calculator_parser_data.cpp
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/calculator_parser_data.h
)

# 创建目录
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# 生成 cc 配置文件
set(SCRIPT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lang.txt")
set(OUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")   # 让生成器输出到构建目录
set(NAMESPACE "cal::gen")
set(NAME "calculator")

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/generate_config.cmake.in
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generate_config.cmake
        @ONLY
)

# 自定义命令：生成源文件
add_custom_command(
        OUTPUT ${GENERATED_FILES}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lang.txt ${SCRIPT}
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generate_config.cmake
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/stupidcoder.cc ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cc.config.json
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        DEPENDS stupidcoder.cc ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generate_config.cmake
        COMMENT "Generating parser data files"
)

# 可执行文件，包含生成的源文件
add_executable(${TARGET} main.cpp ${GENERATED_FILES})

# 包含生成的头文件目录（如果有头文件）
target_include_directories(${TARGET} PRIVATE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# 链接库
target_link_libraries(${TARGET} jyy::common)