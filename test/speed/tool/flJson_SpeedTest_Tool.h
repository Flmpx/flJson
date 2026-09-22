#ifndef FLJSON_SPEEDTEST_TOOL_H
#define FLJSON_SPEEDTEST_TOOL_H

#include <stdlib.h>

// 速度测试总次数
extern int FLJSON_SPEEDTEST_CNT;

extern void TEST_JSON_PARSE_SPEED(const char* jsonSrcDir, size_t parseCnt);

#endif