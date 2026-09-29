cmake_minimum_required(VERSION 3.28.3)

project(flJson)

option(FLJONS_USE_SSH_FOR_GIT OFF)

if (FLJONS_USE_SSH_FOR_GIT)
    set(FLJSON_DEPEND_LINK "git@github.com:Flmpx/hmfocx.git")
else()
    set(FLJSON_DEPEND_LINK "https://github.com/Flmpx/hmfocx.git")
endif()

# 拉取hmfocx库
include(FetchContent)

# 让VS Code的C/C++插件能够识别到头文件, 写代码的时候可以提供代码提示
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
    hmfocx
    GIT_REPOSITORY ${FLJSON_DEPEND_LINK}
    GIT_TAG v0.13.0
)

FetchContent_MakeAvailable(hmfocx)

# 所有的c文件在src目录中
file(GLOB FL_SRC_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(flJson STATIC ${FL_SRC_SOURCE_LIST})

# 只在c文件中使用hmfocx的东西
target_link_libraries(flJson PRIVATE hmfocx)

target_include_directories(flJson PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


option(BUILD_TEST "Run flJson Test" OFF)        #总开关
option(GRAMMAR_TEST "Run GrammarTest" OFF)   
option(SPEED_TEST "Run SpeedTest" OFF)


if (BUILD_TEST)         # 开启测试
    enable_testing()

    if (GRAMMAR_TEST)       # 语法测试
        add_subdirectory(test/grammar)
    endif()

    if (SPEED_TEST)         # 速度测试
        add_subdirectory(test/speed)
    endif()

endif()