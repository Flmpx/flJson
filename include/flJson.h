/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */

#ifndef FL_JSON_H
#define FL_JSON_H

#include <stdlib.h>
#include <stdbool.h>



#define FL_JSON_VERSION_MAJOR 0
#define FL_JSON_VERSION_MINOR 2
#define FL_JSON_VERSION_PATCH 0
#define FL_JSON_VERSION "0.2.0"



/* Json的类型(总共七种) */
typedef enum flJsonType flJsonType;

/* Json本体 */
typedef struct flJson flJson;

/* 全局函数返回状态码 */
typedef enum flRet flRet;

/* 对象迭代器 */
typedef struct flJsonObjectIter flJsonObjectIter;

/* 数组迭代器 */
typedef struct flJsonArrayIter flJsonArrayIter;

enum flJsonType {
    flJsonTypeLL            = 1L << 0,          // 整型
    flJsonTypeDouble        = 1L << 1,          // 浮点型
    flJsonTypeString        = 1L << 2,          // 字符串型
    flJsonTypeNull          = 1L << 3,          // 空
    flJsonTypeBool          = 1L << 4,          // 布尔型
    flJsonTypeObject        = 1L << 5,          // 对象
    flJsonTypeArray         = 1L << 6           // 数组
};

enum flRet {
    flRet_Suc,                  // 成功操作, 比如插入成功
    flRet_Error,                // 重大错误, 比如无法分配内存
    flRet_Warn,                 // 警告, 比如类型错误
    flRet_None,                 // 操作无效, 比如删除不存在的键
};

struct flJson {
    flJsonType type_;        // json的类型标志
    union {
        long long valLL_;  
        double valDouble_;
        char* valString_;    // 字符串时传入的时候会自动生成副本
        bool valBool_;

        /* Json数组 */
        struct {
            /* 只要记录这些的内存地址就行了, hmfocx会自行处理 */
            flJson** array_;      
            size_t size_;
            size_t cap_;
        } valArray_;

        /* Json对象 */
        struct {    
            void* entrys_;
            int* status_;
            size_t size_;
            size_t cap_;
        } valObject_;
    };
    size_t refCount_;         // 引用计数, 当为0时即是释放内存时机
};

struct flJsonObjectIter {
    void* entrys_;
    int* status_;
    size_t cap_;
    size_t idx_;
};

struct flJsonArrayIter {
    flJson** array_;
    size_t size_;
    size_t idx_;
};



/* Json类型的判断 */

extern bool flJson_CheckType(flJson* j, flJsonType type);

/* JsonNull类型的操作 */

extern flJson* flJsonNull_New();

/* JsonLL的操作 */

extern flJson* flJsonLL_New(long long ll);
extern long long* flJsonLL_Get(flJson* jll);

/* JsonDouble的操作 */

extern flJson* flJsonDouble_New(double d);
extern double* flJsonDouble_Get(flJson* jd);

/* JsonBool的操作 */

extern flJson* flJsonBool_New(bool b);
extern bool* flJsonBool_Get(flJson* jb);

/* JsonString的操作 */

extern flJson* flJsonString_New(const char* s);
extern char* flJsonString_Get(flJson* js);

/* JsonArray的操作 */

extern flJson* flJsonArray_New();
extern size_t flJsonArray_Size(flJson* ja);
extern flRet flJsonArray_Add(flJson* ja, flJson* j, size_t idx);
extern flJson* flJsonArray_Get(flJson* ja, size_t idx);
extern flRet flJsonArray_Del(flJson* ja, size_t idx);

extern void flJsonArrayIter_Init(flJsonArrayIter* jai, flJson* ja);
extern bool flJsonArrayIter_HasCur(flJsonArrayIter* jai);
extern flJson* flJsonArrayIter_Cur(flJsonArrayIter* jai);
extern void flJsonArrayIter_MoveNext(flJsonArrayIter* jai);

/* JsonObject的操作 */

extern flJson* flJsonObject_New();
extern size_t flJsonObject_Size(flJson* jo);
extern flRet flJsonObject_Add(flJson* jo, const char* key, flJson* j);
extern flJson* flJsonObject_Get(flJson* jo, const char* key);
extern flRet flJsonObject_Del(flJson* jo, const char* key);

extern void flJsonObjectIter_Init(flJsonObjectIter* joi, flJson* jo);
extern bool flJsonObjectIter_HasCur(flJsonObjectIter* joi);
extern flJson* flJsonObjectIter_CurVal(flJsonObjectIter* joi);
extern const char* flJsonObjectIter_CurKey(flJsonObjectIter* joi);
extern void flJsonObjectIter_MoveNext(flJsonObjectIter* joi);

/* Unref解引Json */

extern void flJson_UnRef(flJson* j);

/* 字符串 --> Json */

extern flJson* flJson_Parse(const char* str);
extern flJson* flJson_ParseWithLength(const char* str, size_t len);

/* Json --> 字符串 */

extern char* flJson_Dump(flJson* j);

#endif
