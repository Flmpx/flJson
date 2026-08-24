#ifndef FL_JSON_H
#define FL_JSON_H

#include <stdlib.h>
#include <stdbool.h>

// Json的类型(总共七种)
typedef enum flJsonType flJsonType;

// Json本体
typedef struct flJson flJson;

// Json数组
typedef struct flArray flArray;

// Json对象
typedef struct flObject flObject;

// 全局函数返回状态码
typedef enum flRet flRet;

enum flJsonType {
    flJsonTypeInt           = 1L << 0,          // 整型
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
    flRet_None,                 // 操作无效, 比如删除不存在的键
};

struct flJson {
    flJsonType type_;        // json的类型标志
    union {
        int valInt_;  
        double valDouble_;
        char* valString_;    // 字符串时传入的时候会自动生成副本
        bool valBool_;
        flArray* valArray_;
        flObject* valObject_;
    };
    size_t refCout_;         // 引用计数, 当为0时即是释放内存时机
};

struct flArray {
    flJson** array_;        // 存着Json指针的数组
    size_t size_;
    size_t cap_;

    size_t refCout_;         // 引用计数, 当为0时即是释放内存时机
};

// Json对象中的条目(包含key和val)
struct flObjectEntry {
    char* key;                  // 键: 字符串
    flJson* json;               // 值: json
};

// 在对象中条目的状态(和hmfocx中的相同)
enum flObjectEntryStatus_ {
    flExisted_,
    flDel_,
    flNone_,
};

struct flObject {
    struct flObjectEntry* entrys_;
    enum flObjectEntryStatus_* status_;
    size_t size_;
    size_t cap_;
    size_t refCout_;         // 引用计数, 当为0时即是释放内存时机
};

/**
 * 创建新不同类型的Json
 */

extern flJson* flJson_NewInt(int i);
extern flJson* flJson_NewDouble(double d);
extern flJson* flJson_NewBool(bool b);
extern flJson* flJson_NewNull();
extern flJson* flJson_NewObject(flObject* o);
extern flJson* flJson_NewArray(flArray* a);
extern flJson* flJson_NewString(const char* s);


/**
 * 判断Json是不是某种类型
 */
extern bool flJson_CheckType(flJson* j, flJsonType type);

/**
 * 获取Json内部的数据
 * 可以通过返回的指针进行修改数据, 返回flArray* , flObject* 的会增加对应的容器内部的引用计数
 * 如果类型不对返回空指针
 */


extern int* flJson_GetInt(flJson* j);
extern double* flJson_GetDouble(flJson* j);
extern char* flJson_GetString(flJson* j);
extern flObject* flJson_GetObject(flJson* j);
extern flArray* flJson_GetArray(flJson* j);
extern bool* flJson_GetBool(flJson* j);


/**
 * Array的相关操作(Get操作会让返回的Json引用加一)
 */

extern flArray* flArray_New();
extern flRet flArray_Add(flArray* a, flJson* j, size_t idx);
extern flRet flArray_Del(flArray* a, size_t idx);
extern flJson* flArray_Get(flArray* a, size_t idx);

/**
 * Object的相关操作(Get操作会让返回的Json引用加一)
 */

extern flObject* flObject_New();
extern flRet flObject_Add(flObject* o, const char* key, flJson* j);
extern flRet flObject_Del(flObject* o, const char* key);
extern flJson* flObject_Get(flObject* o, const char* key);


/**
 * Unref 解除引用
 */

extern void flJson_UnRef(flJson* j);
extern void flArray_UnRef(flArray* a);
extern void flObject_UnRef(flObject* o);


#endif
