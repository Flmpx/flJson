/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */


#ifndef FLJSON_GRAMMARTEST_TOOL_H
#define FLJSON_GRAMMARTEST_TOOL_H

/* 总失败次数 */
extern int FLJSON_GRAMMARTEST_FAIL_CNT;

/* 总成功次数 */
extern int FLJSON_GRAMMARTEST_SUC_CNT;

/* 对还是错 */
typedef enum FL_TAG {
    FL_YES          = 1L << 0,              // 必须是对的
    FL_NO           = 1L << 1               // 必须是错的
} FL_TAG;

extern void TEST_CHECK_JSON_CORRECTNESS(FL_TAG expect, const char* jsonSrcDir);

#endif