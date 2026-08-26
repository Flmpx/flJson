#include "../include/flJson.h"
#include <string.h>
#include <ctype.h>

// 引入hm_str
#include <hm_str.h>


// 当前带解析字符串的状态, head为头指针, now为当前指向
typedef struct strStatus_ {
    const char* const head;
    const char* now;
} strStatus_;


static flJson* flJson_Parse_(strStatus_* status_);   

// 忽略空白字符
static inline void ignoreSpace_(strStatus_* status_) {
    while (isspace(*(status_->now))) status_->now++;
}


// 解析字符串, 异常返回NULL
static char* parseStr_(strStatus_* status_) {
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;   // 创建临时状态信息

    if (*statusTmp_.now == '\0') return NULL;

    if (*statusTmp_.now++ != '\"') return NULL;

    hm_str str;
    if (hm_str_init_reserve(&str, 17) != hm_str_ret_suc) {
        return NULL;
    }

    char tmp[2] = "#";

    while (*statusTmp_.now != '\"') {
        if (*statusTmp_.now == '\0') {
            hm_str_free(&str);
            return NULL;
        }
        if (*statusTmp_.now == '\\') {
            statusTmp_.now++;
            switch (*statusTmp_.now) {
                // 暂时不支持\u
                case 'b':   tmp[0] = '\b'; break;
                case 'f':   tmp[0] = '\f'; break;
                case 'n':   tmp[0] = '\n'; break;
                case 'r':   tmp[0] = '\r'; break;
                case 't':   tmp[0] = '\t'; break;
                case '\"':  tmp[0] = '\"'; break;
                case '\\':  tmp[0] = '\\'; break;
                case 'u':
                default:
                    hm_str_free(&str);
                    return NULL;
            }
        } else {
            tmp[0] = *statusTmp_.now;
        }
        if (hm_str_append(&str, tmp) != hm_str_ret_suc) {
            hm_str_free(&str);
            return NULL;
        }
        statusTmp_.now++;
    }

    statusTmp_.now++;
    status_->now = statusTmp_.now;

    ignoreSpace_(status_);
    return hm_str_pop(&str);
    
}

// 解析整数, 异常返回NULL
static flJson* flJsonLL_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    const char* now = status_->now;

    if (*now == '\0') return NULL;      // 提前判空

    // 前导判断
    const char* tmp = now;
    if (*tmp == '-') tmp++;     // 为负号, 跳过
    if (*tmp >= '0' && *tmp <= '9') {
        // 防止前导0, 比如 `-01`, `001`, `00`
        if (*tmp == '0' && *(tmp + 1) >= '0' && *(tmp + 1) <= '9') {
            return NULL;
        }

    } else {
        // 防止 `-` 这种情况
        return NULL;
    }

    char* end = NULL;

    long long valLL_ = strtoll(status_->now, &end, 10);

    flJson* ret = flJsonLL_New(valLL_);
    
    if (ret == NULL) {
        return NULL;
    }

    status_->now = end;
    ignoreSpace_(status_);

    return ret;

}

// 解析浮点数, 异常返回NULL
static flJson* flJsonDouble_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    const char* now = status_->now;

    if (*now == '\0') return NULL;      // 提前判空

    // 前导判断
    const char* tmp = now;
    if (*tmp == '-') tmp++;     // 为负号, 跳过
    if (*tmp >= '0' && *tmp <= '9') {
        // 防止前导0, 比如 `-01`, `001`, `00`
        if (*tmp == '0' && *(tmp + 1) >= '0' && *(tmp + 1) <= '9') {
            return NULL;
        }

    } else {
        // 防止 `-` 这种情况
        return NULL;
    }

    // 判断.后面必须是数字以及e/E后哦吗必须有至少一个数字(可以有+-)
    while (*tmp != '\0' && strchr("1234567890-+eE.", *tmp) != NULL) {
        if (*tmp == '.') {
            tmp++;
            // 小数点后面必须要有数
            if (!(*tmp >= '0' && *tmp <= '9')) {
                return NULL;
            }
        } else if (*tmp == 'e' || *tmp == 'E') {
            tmp++;
            if (*tmp == '+' || *tmp == '-') tmp++;
            
            // e/E(+-)后面必须要有数
            if (!(*tmp >= '0' && *tmp <= '9')) {
                return NULL;
            }
        }
        tmp++;
    }

    char* end = NULL;
    double valDouble_ = strtod(status_->now, &end);

    flJson* ret = flJsonDouble_New(valDouble_);
    if (ret == NULL) {
        return NULL;
    }
    
    status_->now = end;
    ignoreSpace_(status_);

    return ret;
}


// 解析字符串, 返回flJson, 异常返回NULL
static flJson* flJsonString_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    strStatus_ statusTmp_ = *status_;
    const char* now = status_->now;
    
    if (*now == '\0') return NULL;
    
    char* str = parseStr_(&statusTmp_);

    if (str == NULL) {
        return NULL;
    }
    flJson* ret = flJsonString_New(str);

    if (ret == NULL) {
        free(str);
        return NULL;
    }

    free(str);
    // 只有成功了才可以更新当前指针
    status_->now = statusTmp_.now;
    ignoreSpace_(status_);
    

    return ret;

}

// 解析null, 异常返回NULL
static flJson* flJsonNull_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    const char* now = status_->now;

    if (*now == '\0') return NULL;

    int cmpRes = strncmp(now, "null", 4);

    if (cmpRes != 0) {
        return NULL;
    }

    flJson* ret = flJsonNull_New();

    if (ret == NULL) {
        return NULL;
    }

    status_->now += 4;
    ignoreSpace_(status_);

    return ret;
    
}

// 解析true/false, 异常返回NULL
static flJson* flJsonBool_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    const char* now = status_->now;

    if (*now == '\0') return NULL;

    int cmpRes_1 = strncmp(now, "true", 4);
    int cmpRes_2 = strncmp(now, "false", 5);

    flJson* ret = NULL;
    if (cmpRes_1 == 0) {
        ret = flJsonBool_New(true);
    } else if (cmpRes_2 == 0) {
        ret = flJsonBool_New(false);
    }

    if (ret == NULL) {
        return NULL;
    }

    status_->now += ((cmpRes_1 == 0) ? 4 : 5);

    ignoreSpace_(status_);

    return ret;

}

// 解析Object, 异常返回NULL
static flJson* flJsonObject_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    strStatus_ statusTmp_ = *status_;   // 创建临时状态信息

    if (*statusTmp_.now == '\0') return NULL;

    if (*statusTmp_.now++ != '{') return NULL;

    flJson* obj = flJsonObject_New();
    if (obj == NULL) {
        return NULL;
    }

    ignoreSpace_(&statusTmp_);
    if (*statusTmp_.now == '}') {
        // 空object
        statusTmp_.now++;
        status_->now = statusTmp_.now;
        return obj;
    }

    bool flagError = false;
    do {
        // 解析key
        char* key = parseStr_(&statusTmp_);
        if (key == NULL) {
            flagError = true;
            break;
        }

        ignoreSpace_(&statusTmp_);
        if(*statusTmp_.now != ':') {
            free(key);
            flagError = true;
            break;
        }
        statusTmp_.now++;

        // 解析json
        flJson* j = flJson_Parse_(&statusTmp_);

        if (j == NULL) {
            flagError = true;
            free(key);
            break;
        }

        // 插入
        if (flJsonObject_Add(obj, key, j) == flRet_Suc) {
            free(key);  // 字符串为深拷贝
            flJson_UnRef(j);        // 已经有容器接管了, 可以解引了
        } else {
            free(key); // 字符串为深拷贝
            flJson_UnRef(j);        // 失败了自然要解引
            flagError = true;
            break;
        }
        ignoreSpace_(&statusTmp_);
    } while (*statusTmp_.now == ',' && statusTmp_.now++);   // 如果是逗号, 则跳过, 如果不是, 那个statusTmp_.now是不会加的

    ignoreSpace_(&statusTmp_);
    
    if (flagError || *statusTmp_.now != '}') {
        flJson_UnRef(obj);
        return NULL;
    } else {
        statusTmp_.now++;
        status_->now = statusTmp_.now;
        ignoreSpace_(status_);
        return obj;
    }

}

// 解析Array, 异常返回NULL
static flJson* flJsonArray_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    strStatus_ statusTmp_ = *status_;   // 创建临时状态信息

    if (*statusTmp_.now == '\0') return NULL;

    if (*statusTmp_.now++ != '[') return NULL;

    flJson* arr = flJsonArray_New();
    if (arr == NULL) {
        return NULL;
    }

    ignoreSpace_(&statusTmp_);
    if (*statusTmp_.now == ']') {
        // 空object
        statusTmp_.now++;
        status_->now = statusTmp_.now;
        return arr;
    }

    bool flagError = false;
    do {
        // 解析json
        flJson* j = flJson_Parse_(&statusTmp_);
        if (j == NULL) {
            flagError = true;
            break;
        }

        // 插入
        if (flJsonArray_Add(arr, j, flJsonArray_Size(arr)) == flRet_Suc)  {
            flJson_UnRef(j);
        } else {
            flJson_UnRef(j);
            flagError = true;
            break;
        }
        ignoreSpace_(&statusTmp_);
    } while (*statusTmp_.now == ',' && statusTmp_.now++);   // 如果是逗号, 则跳过, 如果不是, 那个statusTmp_.now是不会加的
    
    ignoreSpace_(&statusTmp_);

    if (flagError || *statusTmp_.now != ']') {
        flJson_UnRef(arr);
        return NULL;
    } else {
        statusTmp_.now++;
        status_->now = statusTmp_.now;
        ignoreSpace_(status_);
        return arr;
    }
    
}

// 解析Json, 异常返回NULL
static flJson* flJson_Parse_(strStatus_* status_) {
    ignoreSpace_(status_);
    const char* now = status_->now;
    char headCh = *now;

    if (headCh == '\0') return NULL;      // 提前判空

    if ((headCh >= '0' && headCh <= '9') || headCh == '-') {
        
        bool doubleFlag = false;
        // 判断是否位浮点数
        for (const char* tmp = now; *tmp != '\0' && strchr("1234567890-+eE.", *tmp) != NULL; tmp++) {
            if (*tmp == 'e' || *tmp == 'E' || *tmp == '.') {
                doubleFlag = true;
                break;
            }
        }
        if (doubleFlag) {
            return flJsonDouble_Parse_(status_);
        } else {
            return flJsonLL_Parse_(status_);
        }
        
    } else if (headCh == 'n') {
        // Null
        return flJsonNull_Parse_(status_);
    } else if (headCh == 't' || headCh == 'f') {
        // Bool
        return flJsonBool_Parse_(status_);
    } else if (headCh == '\"') {
        // String
        return flJsonString_Parse_(status_);
    } else if (headCh == '{') {
        // Object
        return flJsonObject_Parse_(status_);
    } else if (headCh == '[') {
        // Array
        return flJsonArray_Parse_(status_);
    } else {
        // Error
        return NULL;
    }

}


/**
 * 解析字符串为Json
 * 
 * @return - 解析出错返回NULL
 */
flJson* flJson_Parse(const char* str) {
    strStatus_ allStatus = {
        .head = str,
        .now = str
    };
    flJson* ret = flJson_Parse_(&allStatus);

    if (ret == NULL) {
        return NULL;
    } else if (*(allStatus.now) != '\0'){
        flJson_UnRef(ret);
        return NULL;
    } else {
        return ret;
    }
}

