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
static void GRAMMARTEST_FROM_JSONTestSuite() {
    DIR* dir;
    struct dirent* entry;

    const char* skip_json_dirs[] = {
        /* 由于拒绝字符串中的\u0000, 所以这两个文件跳过 */
        "y_object_escaped_null_in_key.json",    // {"foo\u0000bar": 42}
        "y_string_null_escape.json"             // ["\u0000"]
    };
    int skip_json_num = sizeof(skip_json_dirs) / sizeof(const char*);
    

    /* SOURCE_HEAD_PATH  */
    dir = opendir(SOURCE_HEAD_PATH "JSONTestSuite/test_parsing/");
    if (dir == NULL) {
        printf("Can't Open JSONTestSuite/test_parsing/\n");
        return;
    }

    const char* head_dir = "JSONTestSuite/test_parsing/";
    size_t head_dir_len = strlen(head_dir);
    while ((entry = readdir(dir)) != NULL) {
        const char* json_dir = entry->d_name;
        if (strstr(json_dir, ".json") == NULL) {
            continue;
        }

        /* 跳过部分文件 */
        bool flag_skip = false;
        for (int i = 0; i < skip_json_num; i++) {
            if (strcmp(json_dir, skip_json_dirs[i]) == 0) {
                flag_skip = true;
                break;
            }
        }
        if (flag_skip) {
            continue;
        }

        char real_dir[head_dir_len + strlen(json_dir) + 1];
        sprintf(real_dir, "%s%s", head_dir, json_dir);

        FL_TAG expect;
        if (strncmp(json_dir, "y_", 2) == 0) {
            expect = FL_YES;
        } else if (strncmp(json_dir, "n_", 2) == 0) {
            expect = FL_NO;
        } else {
            expect = FL_YES | FL_NO;
        }
        FLJSON_GRAMMARTEST(expect, real_dir);
    } 
    closedir(dir);
}



int main()
{
    GRAMMARTEST_FROM_JSONTestSuite();

    /* 打印总结果 */
    printf("****All: %d,  Passed: %d,  Failed: %d.****\n", FLJSON_GRAMMARTEST_SUC_CNT + FLJSON_GRAMMARTEST_FAIL_CNT, FLJSON_GRAMMARTEST_SUC_CNT, FLJSON_GRAMMARTEST_FAIL_CNT);
    return FLJSON_GRAMMARTEST_FAIL_CNT;
}