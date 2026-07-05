set(TARGET stupidcoder.cc_templates)

add_library(${TARGET} STATIC embedded_templates.cpp)
add_library(jyy::cc_templates ALIAS ${TARGET})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})