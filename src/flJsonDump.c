/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */


#include "../include/flJson.h"
#include <string.h>
#include <assert.h>

/* 引入hm_str hm_map hm_arr */
#include <hm_str.h>
#include <hm_map.h>
#include <hm_arr.h>

/* 用于转化整型和浮点型到字符串的缓冲区大小 */
static const int BUF_SIZE_ = 128;

/* 递归的深度限制 */
static const int MAX_RECURSIVE_DEEPTH_ = 256;

/* 判断递归深度是否过深 */
static inline bool isTooDeep_(int* depth) {
    (*depth)++;
    if (*depth > MAX_RECURSIVE_DEEPTH_) {
        return true;
    } else {
        return false;
    }
}


/* 将json输出到out中 */
static flRet flJson_Dump_(hm_str* out, flJson* j, int depth);

/* 将字符串输出到out中 */
static flRet dumpStr_(hm_str* out, const char* in) {
    if (hm_str_append(out, "\"") != hm_str_ret_suc) {
        return flRet_Error;
    }

    char tmp[3] = "17";
    char ch;
    while (ch = *in) {
        memset(tmp, 0, sizeof(tmp));        /* 全部置零, 以便通过append函数插入 */

        tmp[0] = '\\';
        switch (ch) {
            case '\b': tmp[1] = 'b' ;  break;
            case '\f': tmp[1] = 'f' ;  break;
            case '\n': tmp[1] = 'n' ;  break;
            case '\r': tmp[1] = 'r' ;  break;
            case '\t': tmp[1] = 't' ;  break;
            case '\"': tmp[1] = '\"';  break;
            case '\\': tmp[1] = '\\';  break;
            case '/' : tmp[1] = '/' ;  break;
            default:
                /* 非法字符 */
                if (ch >= 0x00 && ch <= 0x1F) {
                    return flRet_Error;
                }
                tmp[0] = ch;
        }

        if (hm_str_append(out, tmp) != hm_str_ret_suc) {
            return flRet_Error;
        }

        in++;
    }

    if (hm_str_append(out, "\"") != hm_str_ret_suc) {
        return flRet_Error;
    }

    return flRet_Suc;
}

/* 将整型json输出到out中 */
static flRet flJsonLL_Dump_(hm_str* out, flJson* jll) {
    char buf[BUF_SIZE_];
    sprintf(buf, "%lld", jll->valLL_);

    if (hm_str_append(out, buf) != hm_str_ret_suc) {
        return flRet_Error;
    } else {
        return flRet_Suc;
    }
}


/* 将浮点型json输出到out中 */
static flRet flJsonDouble_Dump_(hm_str* out, flJson* jd) {
    char buf[BUF_SIZE_];
    sprintf(buf, "%g", jd->valDouble_);

    if (hm_str_append(out, buf) != hm_str_ret_suc) {
        return flRet_Error;
    } else {
        return flRet_Suc;
    }
}

/* 将字符串类型json输出到out中 */
static flRet flJsonString_Dump_(hm_str* out, flJson* js) {
    return dumpStr_(out, js->valString_);
}

/* 将空类型json输出到out中 */
static flRet flJsonNull_Dump_(hm_str* out, flJson* jn) {
    return hm_str_append(out, "null") != hm_str_ret_suc ? flRet_Error : flRet_Suc;
}

/* 将布尔类型json输出到out中 */
static flRet flJsonBool_Dump_(hm_str* out, flJson* jb) {
    if (jb->valBool_) {
        return hm_str_append(out, "true") != hm_str_ret_suc ? flRet_Error : flRet_Suc;
    } else {
        return hm_str_append(out, "false") != hm_str_ret_suc ? flRet_Error : flRet_Suc;
    }
}

/* 将对象输出到out中, json深度++*/
static flRet flJsonObject_Dump_(hm_str* out, flJson* jo, int depth) {  
    if (isTooDeep_(&depth)) {
        return flRet_Error;
    }

    if (hm_str_append(out, "{") != hm_str_ret_suc) {
        return flRet_Error;
    }

    flJsonObjectIter it;
    flJsonObjectIter_Init(&it, jo);

    size_t i = 0;
    size_t size = flJsonObject_Size(jo);

    while (flJsonObjectIter_HasCur(&it)) {

        /* key */
        if (dumpStr_(out, flJsonObjectIter_CurKey(&it)) != flRet_Suc) {
            return flRet_Error;
        }
        
        /* : */
        if (hm_str_append(out, ":") != hm_str_ret_suc) {
            return flRet_Error;
        }

        /* json */
        if (flJson_Dump_(out, flJsonObjectIter_CurVal(&it), depth) != flRet_Suc) {
            return flRet_Error;
        }

        /* , */
        if (i != size - 1 && hm_str_append(out, ",") != hm_str_ret_suc) {
            return flRet_Error;
        }
        
        i++;
        flJsonObjectIter_MoveNext(&it);
    }

    if (hm_str_append(out, "}") != hm_str_ret_suc) {
        return flRet_Error;
    }
    
    return flRet_Suc;
}

/* 将数组输出到out中, json深度++*/
static flRet flJsonArray_Dump_(hm_str* out, flJson* ja, int depth) {
    if (isTooDeep_(&depth)) {
        return flRet_Error;
    }

    if (hm_str_append(out, "[") != hm_str_ret_suc) {
        return flRet_Error;
    }

    flJsonArrayIter it;
    flJsonArrayIter_Init(&it, ja);

    size_t i = 0;
    size_t size = flJsonArray_Size(ja);

    while (flJsonArrayIter_HasCur(&it)) {

        /* json */
        if (flJson_Dump_(out, flJsonArrayIter_Cur(&it), depth)) {
            return flRet_Error;
        }

        /* , */
        if (i != size - 1 && hm_str_append(out, ",") != hm_str_ret_suc) {
            return flRet_Error;
        }

        i++;
        flJsonArrayIter_MoveNext(&it);
    }

    if (hm_str_append(out, "]") != hm_str_ret_suc) {
        return flRet_Error;
    }
    
    return flRet_Suc;
}


static flRet flJson_Dump_(hm_str* out, flJson* j, int depth) {
    switch (j->type_) {
        case flJsonTypeLL     :  return flJsonLL_Dump_      (out, j);           // Long Long
        case flJsonTypeDouble :  return flJsonDouble_Dump_  (out, j);           // Double
        case flJsonTypeString :  return flJsonString_Dump_  (out, j);           // String
        case flJsonTypeNull   :  return flJsonNull_Dump_    (out, j);           // Null
        case flJsonTypeBool   :  return flJsonBool_Dump_    (out, j);           // Bool
        case flJsonTypeObject :  return flJsonObject_Dump_  (out, j, depth);    // Object
        case flJsonTypeArray  :  return flJsonArray_Dump_   (out, j, depth);    // Array
    }

    return flRet_Error;
}

/**
 * 输出Json为字符串
 * 
 * @return - 输出异常返回NULL
 */
char* flJson_Dump(flJson* j) {
    assert(j != NULL);

    hm_str out;
    if (hm_str_init_reserve(&out, 17) != hm_str_ret_suc) {
        return NULL;
    }

    if (flJson_Dump_(&out, j, 0) != flRet_Suc) {
        hm_str_free(&out);
        return NULL;
    } else {
        return hm_str_pop(&out);
    }
}