/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */

#include "tool/flJson_GrammarTest_Tool.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* 测试来自JSONTestSuite/test_parsing/目录下的json文件 */
static void TEST_JSON_FROM_JSONTestSuite() {
    DIR* dir;
    struct dirent* entry;

    const char* skipJsonFileNames[] = {
        /* 由于拒绝字符串中的\u0000, 所以这两个文件跳过 */
        "y_object_escaped_null_in_key.json",    // {"foo\u0000bar": 42}
        "y_string_null_escape.json"             // ["\u0000"]
    };
    int skipNum = sizeof(skipJsonFileNames) / sizeof(char*);
    

    /* SOURCE_HEAD_PATH  */
    dir = opendir(SOURCE_HEAD_PATH "JSONTestSuite/test_parsing/");
    
    const char* Head = "JSONTestSuite/test_parsing/";
    size_t HeadDirSize = strlen(Head);
    if (dir == NULL) {
        printf("Can't Open JSONTestSuite/test_parsing/\n");
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        const char* fileName = entry->d_name;
        if (strstr(fileName, ".json") == NULL) {
            continue;
        }

        /* 跳过部分文件 */
        bool flag_skip = false;
        for (int i = 0; i < skipNum; i++) {
            if (strcmp(fileName, skipJsonFileNames[i]) == 0) {
                flag_skip = true;
                break;
            }
        }
        if (flag_skip) {
            continue;
        }

        char jsonSrcDir[HeadDirSize + strlen(fileName) + 1];
        sprintf(jsonSrcDir, "%s%s", Head, fileName);

        FL_TAG expect;
        if (strncmp(fileName, "y_", 2) == 0) {
            expect = FL_YES;
        } else if (strncmp(fileName, "n_", 2) == 0) {
            expect = FL_NO;
        } else {
            expect = FL_YES | FL_NO;
        }
        TEST_CHECK_JSON_CORRECTNESS(expect, jsonSrcDir);
    } 
    closedir(dir);

}



int main()
{
    TEST_JSON_FROM_JSONTestSuite();

    /* 打印总结果 */
    printf("****All: %d,  Passed: %d,  Failed: %d.****\n", FLJSON_GRAMMARTEST_SUC_CNT + FLJSON_GRAMMARTEST_FAIL_CNT, FLJSON_GRAMMARTEST_SUC_CNT, FLJSON_GRAMMARTEST_FAIL_CNT);
    return FLJSON_GRAMMARTEST_FAIL_CNT;
}