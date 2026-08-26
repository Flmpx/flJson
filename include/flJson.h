#ifndef FL_JSON_H
#define FL_JSON_H

#include <stdlib.h>
#include <stdbool.h>

// Json的类型(总共七种)
typedef enum flJsonType flJsonType;

// Json本体
typedef struct flJson flJson;

// 全局函数返回状态码
typedef enum flRet flRet;

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

        // Json数组
        struct {
            flJson** array_;        // 存着Json指针的数组
            size_t size_;
            size_t cap_;
        } valArray_;

        // Json对象
        struct {    
            // 只要记录这些的内存地址就行了, hmfocx会自行处理
            void* entrys_;
            int* status_;
            size_t size_;
            size_t cap_;
        } valObject_;
    };
    size_t refCount_;         // 引用计数, 当为0时即是释放内存时机
};


// 不同类型Json的创建

extern flJson* flJsonLL_New(long long ll);
extern flJson* flJsonDouble_New(double d);
extern flJson* flJsonBool_New(bool b);
extern flJson* flJsonNull_New();
extern flJson* flJsonObject_New();
extern flJson* flJsonArray_New();
extern flJson* flJsonString_New(const char* s);


// Json类型的判断

extern bool flJson_CheckType(flJson* j, flJsonType type);


// JsonLL的操作

extern long long* flJsonLL_Get(flJson* jll);


// JsonDouble的操作

extern double* flJsonDouble_Get(flJson* jd);

// JsonBool的操作

extern bool* flJsonBool_Get(flJson* jb);

// JsonString的操作

extern char* flJsonString_Get(flJson* js);

// JsonArray的操作

extern size_t flJsonArray_Size(flJson* ja);
extern flRet flJsonArray_Add(flJson* ja, flJson* j, size_t idx);
extern flJson* flJsonArray_Get(flJson* ja, size_t idx);
extern flRet flJsonArray_Del(flJson* ja, size_t idx);

// JsonObject的操作

extern size_t flJsonObject_Size(flJson* jo);
extern flRet flJsonObject_Add(flJson* jo, const char* key, flJson* j);
extern flJson* flJsonObject_Get(flJson* jo, const char* key);
extern flRet flJsonObject_Del(flJson* jo, const char* key);


// Unref解引Json

extern void flJson_UnRef(flJson* j);


// 字符串 --> Json

extern flJson* flJson_Parse(const char* str);

#endif
