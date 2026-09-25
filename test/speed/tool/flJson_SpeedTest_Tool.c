#define _POSIX_C_SOURCE 199309L

#include <flJson.h>
#include "flJson_SpeedTest_Tool.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int FLJSON_SPEEDTEST_CNT = 0;

/* 颜色 */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"

/* 文件打开失败 */
static void PRINT_OPENFILE_FAIL(const char* jsonSrcDir) {
    printf(COLOR_YELLOW "Can't open %s" COLOR_RESET "\n", jsonSrcDir);
}

/* 复制文内容到字符串失败 */
static void PRINT_MALLOC_FAIL(const char* jsonSrcDir) {
    printf(COLOR_YELLOW "Malloc failed when copy %s" COLOR_RESET "\n", jsonSrcDir);
}

/* 获取当前的时间(单位: 毫秒) */
static double GET_TIME_IN_MS() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

/* 打印速度的结果信息 */
static void PRINT_SPEED_RESULT(double timeDiff, size_t fileSize, size_t runCount, const char* jsonSrcDir) {
    printf( COLOR_GREEN "%s" COLOR_RESET "\n", jsonSrcDir);
    
    double size = (double)fileSize / 1024.0 / 1024.0;
    double time = timeDiff / 1e3;

    printf("| Size     : %gMB   \n", size);
    printf("| Oper Cnt : %zu    \n", runCount);
    printf("| Cost Tiem: %gs    \n", time);
    printf("| Speed    : %gMB/S \n", size * runCount / time);
    printf("\n");

    FLJSON_SPEEDTEST_CNT++;
}

/* 测试解析速度, 路径必须是基于test/speed目录下的, parseCnt是需要解析的次数 */
void TEST_JSON_PARSE_SPEED(const char* jsonSrcDir, size_t parseCnt) {

    /* 由于路径问题, 所有需要修改文件路径 */
    size_t headDirSize = strlen(SOURCE_HEAD_PATH);       // FLJSON_SOURCE_HEAD_PATH 是当前整个项目的test/文件夹的绝对路径
    size_t tailDirSize = strlen(jsonSrcDir);
    char realJsonSrcDir[headDirSize + tailDirSize + 1];
    sprintf(realJsonSrcDir,  SOURCE_HEAD_PATH "%s", jsonSrcDir);

    /* 打开文件 */
    FILE* jsonFile = fopen(realJsonSrcDir, "rb");
    if (jsonFile == NULL) {
        PRINT_OPENFILE_FAIL(realJsonSrcDir);
        return;
    }

    /* 复制内容到字符串中 */
    fseek(jsonFile, 0, SEEK_END);
    long jsonFileSize = ftell(jsonFile);
    fseek(jsonFile, 0, SEEK_SET);
    char* jsonStr = malloc(jsonFileSize + 1);
    if (jsonStr == NULL) {
        PRINT_MALLOC_FAIL(jsonSrcDir);
    }
    fread(jsonStr, 1, jsonFileSize, jsonFile);
    jsonStr[jsonFileSize] = '\0';

    /* 解析json字符串 */
    flJson* roots[parseCnt];     // 为了只测试解析速度而不加入释放所占据的时间
    double timeStart = GET_TIME_IN_MS();
    for (size_t i = 0; i < parseCnt; i++) {
        roots[i] = flJson_ParseWithLength(jsonStr, jsonFileSize);
    }
    double timeEnd = GET_TIME_IN_MS();

    PRINT_SPEED_RESULT(timeEnd - timeStart, jsonFileSize, parseCnt, jsonSrcDir);

    /* 清理资源 */
    fclose(jsonFile);
    for (size_t i = 0; i < parseCnt; i++) {
        flJson_UnRef(roots[i]);
    }
    free(jsonStr);
}