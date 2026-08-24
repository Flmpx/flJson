cmake_minimum_required(VERSION 3.28.3)

project(flJson)


# 拉取hmfocx库
include(FetchContent)

# 让VS Code的C/C++插件能够识别到头文件, 写代码的时候可以提供代码提示
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
    hmfocx
    GIT_REPOSITORY git@github.com:Flmpx/hmfocx.git
    GIT_TAG v0.13.0
)

FetchContent_MakeAvailable(hmfocx)

set(CMAKE_BUILD_TYPE Debug)

# 所有的c文件在src目录中
file(GLOB FL_SRC_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(flJson STATIC ${FL_SRC_SOURCE_LIST})

# 只在c文件中使用hmfocx的东西
target_link_libraries(flJson PRIVATE hmfocx)

target_include_directories(flJson PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)