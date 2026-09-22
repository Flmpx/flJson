#include "tool/flJson_SpeedTest_Tool.h"
#include <stdio.h>

// 测试来自 https://jsonconsole.com/sample-json/large-5mb 目录下的json文件
static void TEST_JSON_FROM_jsonconsole() {
    const char* fileDir = "jsonconsole/large-5mb.json";
    TEST_JSON_PARSE_SPEED(fileDir, 100);
}

int main()
{
    TEST_JSON_FROM_jsonconsole();

    // 打印结果
    printf("****All: %d.****\n", FLJSON_SPEEDTEST_CNT);
    return 0;
}