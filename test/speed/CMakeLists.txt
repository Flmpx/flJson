cmake_minimum_required(VERSION 3.28.3)

# 速度测试
set(flJSON_TARGET flJson_SpeedTest)

project(${flJSON_TARGET})

# 测试文件
set(${flJSON_TARGET}_SRC ${CMAKE_CURRENT_SOURCE_DIR}/${flJSON_TARGET}.c)

# 引入测试工具函数
set(${flJSON_TARGET}_TOOL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tool/${flJSON_TARGET}_Tool.c)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 在flJsontest.c文件中些的目录都必须是基于flJson/test/speed/目录的
set(${flJSON_TARGET}_DATA_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${flJSON_TARGET}_Exe "${${flJSON_TARGET}_SRC}" "${${flJSON_TARGET}_TOOL_SRC}")

# 直接使用flJson
target_link_libraries(${flJSON_TARGET}_Exe PRIVATE flJson)

# SOURCE_HEAD_PATH 宏为当前flJson/test/speed/目录的绝对路径
target_compile_definitions(${flJSON_TARGET}_Exe PRIVATE SOURCE_HEAD_PATH="${${flJSON_TARGET}_DATA_SRC_DIR}/")

# 测试
add_test(NAME ${flJSON_TARGET}_Exe COMMAND ${flJSON_TARGET}_Exe)
