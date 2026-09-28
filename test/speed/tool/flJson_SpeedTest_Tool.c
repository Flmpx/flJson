/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */

#define _POSIX_C_SOURCE 199309L

#include <flJson.h>
#include "flJson_SpeedTest_Tool.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int FLJSON_SPEEDTEST_PARSE_CNT = 0;

/* 颜色 */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"

/* 文件打开失败 */
static void PRINT_OPENFILE_FAIL(const char* json_dir) {
    printf(COLOR_YELLOW "Can't open %s" COLOR_RESET "\n", json_dir);
}

/* 复制文内容到字符串失败 */
static void PRINT_MALLOC_FAIL(const char* json_dir) {
    printf(COLOR_YELLOW "Malloc failed when copy %s" COLOR_RESET "\n", json_dir);
}

/* 获取当前的时间(单位: 毫秒) */
static double GET_TIME_IN_MS() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

/* 打印速度的结果信息 */
static void PRINT_SPEED_RESULT(double timeDiff, size_t fileSize, size_t runCount, const char* json_dir) {
    printf( COLOR_GREEN "%s" COLOR_RESET "\n", json_dir);
    
    double size = (double)fileSize / 1024.0 / 1024.0;
    double time = timeDiff / 1e3;

    printf("| Size     : %gMB   \n", size);
    printf("| Oper Cnt : %zu    \n", runCount);
    printf("| Cost Time: %gs    \n", time);
    printf("| Speed    : %gMB/S \n", size * runCount / time);
    printf("\n");

    FLJSON_SPEEDTEST_PARSE_CNT++;
}

/* 测试解析速度, 路径必须是基于test/speed目录下的, parse_cnt是需要解析的次数 */
void FLJSON_SPEEDTEST_PARSE(const char* json_dir, size_t parse_cnt) {

    /* 由于路径问题, 所有需要修改文件路径 */
    size_t head_dir_len = strlen(SOURCE_HEAD_PATH);       // SOURCE_HEAD_PATH 是当前整个项目的test/speed文件夹的绝对路径
    size_t json_dir_len = strlen(json_dir);
    char real_dir[head_dir_len + json_dir_len + 1];
    sprintf(real_dir,  SOURCE_HEAD_PATH "%s", json_dir);

    /* 打开文件 */
    FILE* json_file = fopen(real_dir, "rb");
    if (json_file == NULL) {
        PRINT_OPENFILE_FAIL(real_dir);
        return;
    }

    /* 复制内容到字符串中 */
    fseek(json_file, 0, SEEK_END);
    long json_file_len = ftell(json_file);
    fseek(json_file, 0, SEEK_SET);
    char* json_str = malloc(json_file_len + 1);
    if (json_str == NULL) {
        PRINT_MALLOC_FAIL(json_dir);
    }
    fread(json_str, 1, json_file_len, json_file);
    json_str[json_file_len] = '\0';

    /* 解析json字符串 */
    flJson* roots[parse_cnt];     // 为了只测试解析速度而不加入释放所占据的时间

    double start_time = GET_TIME_IN_MS();
    for (size_t i = 0; i < parse_cnt; i++) {
        roots[i] = flJson_ParseWithLength(json_str, json_file_len);
    }
    double end_time = GET_TIME_IN_MS();

    PRINT_SPEED_RESULT(end_time - start_time, json_file_len, parse_cnt, json_dir);

    /* 清理资源 */
    fclose(json_file);
    for (size_t i = 0; i < parse_cnt; i++) {
        flJson_UnRef(roots[i]);
    }
    free(json_str);
}