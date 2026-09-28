/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */

#include "tool/flJson_SpeedTest_Tool.h"
#include <stdio.h>

/* 测试来自 https://jsonconsole.com/sample-json/large-5mb 目录下的json文件 */
static void SPEEDTEST_PARSE_FROM_jsonconsole() {
    const char* json_dir = "jsonconsole/large-5mb.json";
    FLJSON_SPEEDTEST_PARSE(json_dir, 100);
}

int main()
{
    SPEEDTEST_PARSE_FROM_jsonconsole();

    /* 打印结果 */
    printf("****All: %d, SpeedTest: %d.****\n", FLJSON_SPEEDTEST_PARSE_CNT, FLJSON_SPEEDTEST_PARSE_CNT);
    return 0;
}