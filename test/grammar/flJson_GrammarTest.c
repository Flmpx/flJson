#include "tool/flJson_GrammarTest_Tool.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

// 测试来自JSONTestSuite/test_parsing/目录下的json文件
static void TEST_JSON_FROM_JSONTestSuite() {
    DIR* dir;
    struct dirent* entry;

    // SOURCE_HEAD_PATH 
    const char* Head = SOURCE_HEAD_PATH "JSONTestSuite/test_parsing/";
    size_t HeadDirSize = strlen(Head);
    dir = opendir(Head);
    if (dir == NULL) {
        printf("Can't Open " SOURCE_HEAD_PATH "JSONTestSuite/test_parsing/\n");
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        const char* fileName = entry->d_name;
        if (strstr(fileName, ".json") == NULL) {
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

}



int main()
{
    TEST_JSON_FROM_JSONTestSuite();

    return FLJSONTEST_FAIL_CNT;
}