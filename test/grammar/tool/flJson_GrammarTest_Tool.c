#include <stdio.h>
#include <string.h>
#include <flJson.h>
#include "flJson_GrammarTest_Tool.h"

int FLJSON_GRAMMARTEST_FAIL_CNT = 0;
int FLJSON_GRAMMARTEST_SUC_CNT = 0;

// 颜色
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"

// 打印的时候会使用这些符号
static const char* FL_TAG_YES_STR    =   "✅";
static const char* FL_TAG_NO_STR     =   "❌";
static const char* FL_TAG_IMPLE_STR  =   "❔";



// 根据tag返回对应的全局字符串
static const char* GET_TAG_STR(FL_TAG tag) {
    if (tag == FL_YES) return FL_TAG_YES_STR;
    else if (tag == FL_NO) return FL_TAG_NO_STR;
    else  return FL_TAG_IMPLE_STR;
}

// 文件打开失败
static void PRINT_OPENFILE_FAIL(const char* jsonSrcDir) {
    printf(COLOR_YELLOW "Can't open %s" COLOR_RESET "\n", jsonSrcDir);
}

// 复制文内容到字符串失败
static void PRINT_MALLOC_FAIL(const char* jsonSrcDir) {
    printf(COLOR_YELLOW "Malloc failed when copy %s" COLOR_RESET "\n", jsonSrcDir);
}


// 打印结果信息
static void PRINT_CHECK_JSON_RESULT(FL_TAG expect, FL_TAG real, const char* jsonSrcDir) {
    printf("|EXPECT:%s |REAL:%s|", GET_TAG_STR(expect), GET_TAG_STR(real));
    printf(" --- ");
    if (expect & real) {
        printf(COLOR_GREEN);
        FLJSON_GRAMMARTEST_SUC_CNT++;
    } else {
        printf(COLOR_RED);
        FLJSON_GRAMMARTEST_FAIL_CNT++;
    }
    printf("%s\n", jsonSrcDir);
    printf(COLOR_RESET);
}

// 对json进行测试, 路径必须是基于test/grammar目录下的
void TEST_CHECK_JSON_CORRECTNESS(FL_TAG expect, const char* jsonSrcDir) {

    // 由于路径问题, 所有需要修改文件路径
    size_t headDirSize = strlen(SOURCE_HEAD_PATH);       // FLJSON_SOURCE_HEAD_PATH 是当前整个项目的test/文件夹的绝对路径
    size_t tailDirSize = strlen(jsonSrcDir);
    char realJsonSrcDir[headDirSize + tailDirSize + 1];
    sprintf(realJsonSrcDir,  SOURCE_HEAD_PATH "%s", jsonSrcDir);
    
    // 打开文件
    FILE* jsonFile = fopen(realJsonSrcDir, "rb");
    if (jsonFile == NULL) {
        PRINT_OPENFILE_FAIL(realJsonSrcDir);
        return;
    }

    // 复制内容到字符串中
    fseek(jsonFile, 0, SEEK_END);
    long jsonFileSize = ftell(jsonFile);
    fseek(jsonFile, 0, SEEK_SET);
    char* jsonStr = malloc(jsonFileSize + 1);
    if (jsonStr == NULL) {
        PRINT_MALLOC_FAIL(jsonSrcDir);
    }
    fread(jsonStr, 1, jsonFileSize, jsonFile);
    jsonStr[jsonFileSize] = '\0';
    
    // 解析json字符串
    flJson* root = flJson_ParseWithLength(jsonStr, jsonFileSize);
    PRINT_CHECK_JSON_RESULT(expect, root == NULL ? FL_NO : FL_YES, jsonSrcDir);
    
    // 清理资源
    fclose(jsonFile);
    flJson_UnRef(root);
    free(jsonStr);
}
