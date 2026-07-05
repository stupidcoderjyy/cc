# 定义目标目录
set(TEMPLATE_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/templates)

# 创建目录
file(MAKE_DIRECTORY ${TEMPLATE_DIR})

# 创建自定义目标，执行文件操作
add_custom_target(prepare_templates
        COMMAND ${CMAKE_COMMAND} -E remove ${TEMPLATE_DIR}/parser_data.h.txt ${TEMPLATE_DIR}/parser_data.h.txt
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/parser_data.h.txt ${TEMPLATE_DIR}/parser_data.h.txt
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/parser_data.cpp.txt ${TEMPLATE_DIR}/parser_data.cpp.txt
        COMMENT "Copying template files to ${TEMPLATE_DIR}"
)