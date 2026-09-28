/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */

#ifndef FLJSON_SPEEDTEST_TOOL_H
#define FLJSON_SPEEDTEST_TOOL_H

#include <stdlib.h>

/* 解析速度测试总次数 */
extern int FLJSON_SPEEDTEST_PARSE_CNT;

extern void FLJSON_SPEEDTEST_PARSE(const char* json_dir, size_t parse_cnt);

#endif