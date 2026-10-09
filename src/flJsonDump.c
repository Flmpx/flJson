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
    if (hm_str_append_ch(out, '\"') != hm_str_ret_suc) {
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

        if (hm_str_append(out, tmp, strlen(tmp)) != hm_str_ret_suc) {
            return flRet_Error;
        }

        in++;
    }

    if (hm_str_append_ch(out, '\"') != hm_str_ret_suc) {
        return flRet_Error;
    }

    return flRet_Suc;
}

/* 将整型json输出到out中 */
static flRet flJsonLL_Dump_(hm_str* out, flJson* jll) {
    char buf[BUF_SIZE_];
    sprintf(buf, "%lld", jll->valLL_);

    if (hm_str_append(out, buf, strlen(buf)) != hm_str_ret_suc) {
        return flRet_Error;
    } else {
        return flRet_Suc;
    }
}


/* 将浮点型json输出到out中 */
static flRet flJsonDouble_Dump_(hm_str* out, flJson* jd) {
    char buf[BUF_SIZE_];
    sprintf(buf, "%lf", jd->valDouble_);

    if (hm_str_append(out, buf, strlen(buf)) != hm_str_ret_suc) {
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
    return hm_str_append(out, "null", 4) != hm_str_ret_suc ? flRet_Error : flRet_Suc;
}

/* 将布尔类型json输出到out中 */
static flRet flJsonBool_Dump_(hm_str* out, flJson* jb) {
    if (jb->valBool_) {
        return hm_str_append(out, "true", 4) != hm_str_ret_suc ? flRet_Error : flRet_Suc;
    } else {
        return hm_str_append(out, "false", 5) != hm_str_ret_suc ? flRet_Error : flRet_Suc;
    }
}

/* 将对象输出到out中, json深度++*/
static flRet flJsonObject_Dump_(hm_str* out, flJson* jo, int depth) {  
    if (isTooDeep_(&depth)) {
        return flRet_Error;
    }

    if (hm_str_append_ch(out, '{') != hm_str_ret_suc) {
        return flRet_Error;
    }

    hm_map_iter iter = {
        .buckets = jo->valObject_.entrys_,
        .buckets_status = (hm_map_entry_status*)jo->valObject_.status_,
        .index = 0,
        .len = jo->valObject_.cap_
    };

    size_t i = 0;
    size_t size = jo->valObject_.size_;

    while (hm_map_iter_has_cur(&iter)) {
        hm_map_entry e = hm_map_iter_cur(&iter);

        /* key */
        if (dumpStr_(out, e.key) != flRet_Suc) {
            return flRet_Error;
        } 

        /* : */
        if (hm_str_append_ch(out, ':') != hm_str_ret_suc) {
            return flRet_Error;
        }

        /* json */
        if (flJson_Dump_(out, e.val, depth) != flRet_Suc) {
            return flRet_Error;
        }

        /* , */
        if (i != size - 1 && hm_str_append_ch(out, ',') != hm_str_ret_suc) {
            return flRet_Error;
        }
        
        i++;
        hm_map_iter_move_next(&iter);
    }

    if (hm_str_append_ch(out, '}') != hm_str_ret_suc) {
        return flRet_Error;
    }
    
    return flRet_Suc;
}

/* 将数组输出到out中, json深度++*/
static flRet flJsonArray_Dump_(hm_str* out, flJson* ja, int depth) {
    if (isTooDeep_(&depth)) {
        return flRet_Error;
    }

    if (hm_str_append_ch(out, '[') != hm_str_ret_suc) {
        return flRet_Error;
    }

    size_t size = ja->valArray_.size_;
    flJson** vals = (flJson**)ja->valArray_.array_;

    for (size_t i = 0; i < size; i++) {
        
        /* json */
        if (flJson_Dump_(out, vals[i], depth)) {
            return flRet_Error;
        }

        /* , */
        if (i != size - 1 && hm_str_append_ch(out, ',') != hm_str_ret_suc) {
            return flRet_Error;
        }
    }

    if (hm_str_append_ch(out, ']') != hm_str_ret_suc) {
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
    if (hm_str_init(&out) != hm_str_ret_suc) {
        return NULL;
    }

    if (flJson_Dump_(&out, j, 0) != flRet_Suc) {
        hm_str_free(&out);
        return NULL;
    } else {
        return hm_str_pop(&out);
    }
}