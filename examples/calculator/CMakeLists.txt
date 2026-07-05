set(TARGET example_calculator)
set(SRC
        gen/calculator_parser_data.cpp
        main.cpp
)

add_executable(${TARGET} ${SRC})

# 步骤3：替换 @TEMPLATE@、@NAMESPACE@、@DEST@、@NAME@，生成 cc.config.json.in
set(SCRIPT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lang.txt")
set(OUT "${CMAKE_CURRENT_SOURCE_DIR}/gen")
set(NAMESPACE "cal::gen")
set(NAME "calculator")
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cc.config.json.in
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cc.config.json
        @ONLY
)

# 步骤1、2、4：自定义目标，在构建 TARGET 之前执行
add_custom_target(generate_config
        COMMAND ${CMAKE_COMMAND} -E remove ${SCRIPT}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lang.txt ${SCRIPT}
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/stupidcoder.cc ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cc.config.json --debug
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        DEPENDS stupidcoder.cc ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cc.config.json
        COMMENT "Preparing lang.txt and running cc generator"
)

# 使目标依赖于 generate_config，确保前置步骤先执行
add_dependencies(${TARGET} generate_config)

target_link_libraries(${TARGET}
        jyy::common
)