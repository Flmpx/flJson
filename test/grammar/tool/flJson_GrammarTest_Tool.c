/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */

#include <stdio.h>
#include <string.h>
#include <flJson.h>
#include "flJson_GrammarTest_Tool.h"

int FLJSON_GRAMMARTEST_FAIL_CNT = 0;
int FLJSON_GRAMMARTEST_SUC_CNT  = 0;

/* 颜色 */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"

/* 打印的时候会使用这些符号 */
static const char* FL_TAG_YES_STR    =   "✅";
static const char* FL_TAG_NO_STR     =   "❌";
static const char* FL_TAG_IMPLE_STR  =   "❔";



/* 根据tag返回对应的全局字符串 */
static const char* GET_TAG_STR(FL_TAG tag) {
    if (tag == FL_YES) return FL_TAG_YES_STR;
    else if (tag == FL_NO) return FL_TAG_NO_STR;
    else  return FL_TAG_IMPLE_STR;
}

/* 文件打开失败 */
static void PRINT_OPENFILE_FAIL(const char* json_dir) {
    printf(COLOR_YELLOW "Can't open %s" COLOR_RESET "\n", json_dir);
}

/* 复制文内容到字符串失败 */
static void PRINT_MALLOC_FAIL(const char* json_dir) {
    printf(COLOR_YELLOW "Malloc failed when copy %s" COLOR_RESET "\n", json_dir);
}


/* 打印结果信息 */
static void PRINT_CHECK_RESULT(FL_TAG expect, FL_TAG real, const char* json_dir) {
    printf("|EXPECT:%s |REAL:%s|", GET_TAG_STR(expect), GET_TAG_STR(real));
    printf(" --- ");
    if (expect & real) {
        printf(COLOR_GREEN);
        FLJSON_GRAMMARTEST_SUC_CNT++;
    } else {
        printf(COLOR_RED);
        FLJSON_GRAMMARTEST_FAIL_CNT++;
    }
    printf("%s\n", json_dir);
    printf(COLOR_RESET);
}

/* 对json进行语法测试, json文件路径必须是基于test/grammar目录下的 */
void FLJSON_GRAMMARTEST(FL_TAG expect, const char* json_dir) {

    /* 由于路径问题, 所有需要修改文件路径 */
    size_t head_dir_len = strlen(SOURCE_HEAD_PATH);       // SOURCE_HEAD_PATH 是当前整个项目的test/grammar/文件夹的绝对路径
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
    flJson* root = flJson_ParseWithLength(json_str, json_file_len);
    PRINT_CHECK_RESULT(expect, root == NULL ? FL_NO : FL_YES, json_dir);
    
    /* 清理资源 */
    fclose(json_file);
    flJson_UnRef(root);
    free(json_str);
}
