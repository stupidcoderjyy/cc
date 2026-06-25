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

add_subdirectory(thirdparty)
add_subdirectory(src)
add_subdirectory(test)