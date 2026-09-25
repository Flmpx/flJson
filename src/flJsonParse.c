#include "../include/flJson.h"
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* 引入hm_str */
#include <hm_str.h>

/****************************************************************************
 * 相关解析函数(中转函数除外)均采用创建带解析字符串的临时状态, 然后先进行解析
 * 如果解析失败, 不更新传入的带解析的字符串状态, 若成功则更新
 * 
 * 在解析的过程中, 如果要查看下一个或一段字符的内容, 首先要进行判断是否越界
 * 比如解析 `true` 那就的在解析之前写上 `if (now + 4 > tail) return NULL;`
 * 
 * 在每个函数开头基本都会有一个经典开头: 判断递归深度, 跳过空白字符
 ***************************************************************************/


static const int maxRecusiveDepth_ = 256;

/* 当前待解析字符串的状态, tail为尾指针, now为当前指向 */
typedef struct strStatus_ {
    const char* const tail;
    const char* now;
} strStatus_;


static flJson* flJson_Parse_(strStatus_* status_, int depth);   

/* 忽略空白字符 */
static inline void ignoreSpace_(strStatus_* status_) {
    while (status_->now < status_->tail && 
          (*(status_->now) == '\n' || *(status_->now) == '\t' || *(status_->now) == '\n' || *(status_->now) == ' ' || *(status_->now) == '\r')) status_->now++;
}

static inline bool isTooDeep(int* depth) {
    (*depth)++;
    if (*depth > maxRecusiveDepth_) {
        return true;
    } else {
        return false;
    }
}

/* 16进制字符转为10进制, 若字符不合法, 返回-1 */
static int hexToInt(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

/* Unicode码点转UTF-8字节序列, 必须保证out字符串开始的时候全为 '\0', 返回写入的字符数 */
static int codePointToUtf8(uint32_t cp, char* out, int depth) {
    if (isTooDeep(&depth)) {
        return -1;
    }

    if (cp == 0x0000    ||         // flJson拒绝\u0000
        cp > 0x10FFFF   ||          
       (cp >= 0xD800 && cp <= 0xDFFF)) {
        return 0;
    }

    if (cp <= 0x7F) {
        out[0] = (char)cp;

        return 1;
    } else if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
        out[1] = (char)(0x80 | (cp & 0x3F));

        return 2;
    } else if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));

        return 3;
    } else if (cp <= 0x10FFFF) {
        out[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));

        return 4;
    }

}

/* 解析字符串中的Unicode码点, 需要保证in全是 '\0', 解析出错返回NULL, 正确就返回in本身 */
static char* ParseUnicode_(strStatus_* status_, char* in, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    
    /* 由于是在字符串里面, 不可以跳过空白字符 */

    strStatus_ statusTmp_ = *status_;   // 创建临时状态信息

    if (statusTmp_.now + 6 > statusTmp_.tail) return NULL;  // \u0000为六个字符

    if (*statusTmp_.now != '\\') return NULL;
    statusTmp_.now++;
    if (*statusTmp_.now != 'u') return NULL;
    statusTmp_.now++;

    uint32_t codePoint = 0;
    for (int i = 0; i < 4; i++) {
        int v = hexToInt(*statusTmp_.now++);
        if (v == -1) {
            return NULL;
        }
        codePoint = (codePoint << 4) | v; 
    }

    /* 高位标记 */
    if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {

        if (statusTmp_.now + 6 > statusTmp_.tail) return NULL;

        if (*statusTmp_.now != '\\') return NULL;
        statusTmp_.now++;    
        if (*statusTmp_.now != 'u') return NULL;
        statusTmp_.now++;
        
        uint32_t low = 0;
        for (int i = 0; i < 4; i++) {
            int v = hexToInt(*statusTmp_.now++);
            if (v == -1) {
                return NULL;
            }
            low = (low << 4) | v; 
        }

        if (!(low >= 0xDC00 && low <= 0xDFFF)) {
            return NULL;
        }

        codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);

    } else if (codePoint >= 0xDC00 && codePoint <= 0xDFFF) {        // 孤立的低位
        return NULL;
    }

    if (codePointToUtf8(codePoint, in, depth) <= 0) {
        return NULL;
    }

    /* 解析成功, 更新状态并返回 */
    status_->now = statusTmp_.now;
    return in;
}

/* 解析字符串, 异常返回NULL */
static char* parseStr_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);


    strStatus_ statusTmp_ = *status_;   // 创建临时状态信息

    if (statusTmp_.now + 1 > statusTmp_.tail) return NULL;

    if (*statusTmp_.now != '\"') return NULL;
    statusTmp_.now++;

    if (statusTmp_.now + 1 > statusTmp_.tail) return NULL;

    hm_str str;
    if (hm_str_init_reserve(&str, 17) != hm_str_ret_suc) {
        return NULL;
    }

    char tmp[5] = "5201";       // 转码最多4个字节

    while (*statusTmp_.now != '\"') {
        memset(tmp, 0, sizeof(tmp));        // 全部置零, 以便通过append函数插入
        
        /* 非法字符 */
        if (*statusTmp_.now >= 0x00 && *statusTmp_.now <= 0x1F) {
            hm_str_free(&str);
            return NULL;
        }

        /* 发现转义 */
        if (*statusTmp_.now == '\\') {

            statusTmp_.now++;
            if (statusTmp_.now + 1 > statusTmp_.tail) {
                hm_str_free(&str);
                return NULL;
            }

            switch (*statusTmp_.now) {
                case 'b':   tmp[0] = '\b'; break;
                case 'f':   tmp[0] = '\f'; break;
                case 'n':   tmp[0] = '\n'; break;
                case 'r':   tmp[0] = '\r'; break;
                case 't':   tmp[0] = '\t'; break;
                case '\"':  tmp[0] = '\"'; break;
                case '\\':  tmp[0] = '\\'; break;
                case '/':   tmp[0] = '/' ; break;
                case 'u':
                    statusTmp_.now--;       // 退回到\, 函数parseUnicode_会检查是否有\u
                    if (ParseUnicode_(&statusTmp_, tmp, depth) == NULL) {
                        hm_str_free(&str);
                        return NULL;
                    }
                    statusTmp_.now--;       //由于ParseUnicode_函数会跳到\uxxxx的后面去, 所以要跳回来
                break;
                default:
                    hm_str_free(&str);
                    return NULL;
            }
        } else {
            tmp[0] = *statusTmp_.now;
        }

        /* 拼接字符串 */
        if (hm_str_append(&str, tmp) != hm_str_ret_suc) {
            hm_str_free(&str);
            return NULL;
        }

        statusTmp_.now++;           // 当前字符解析完成, 跳到下一个
        if (statusTmp_.now + 1 > statusTmp_.tail) {
            hm_str_free(&str);
            return NULL;
        }

    }

    statusTmp_.now++;
    
    /* 解析成功, 更新状态并返回 */
    status_->now = statusTmp_.now;
    return hm_str_pop(&str);
}

/* 解析整数, 异常返回NULL */
static flJson* flJsonLL_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;

    if (statusTmp_.now + 1 > statusTmp_.tail) return NULL;
    
    
    /* 由于strtoll函数没法根据len来解析数字, 故创建小型缓冲区 */
    const char* tmp = statusTmp_.now;
    while (tmp < statusTmp_.tail && 
          ((*tmp >= '0' && *tmp <= '9') || *tmp == '-')) {
            tmp++;
    }
    char* start = (char*)malloc(tmp - statusTmp_.now + 1);
    if (start == NULL) {
        return NULL;
    }
    memcpy(start, statusTmp_.now, tmp - statusTmp_.now);
    start[tmp - statusTmp_.now] = '\0';


    
    const char* now = start;

    /* 前导判断 */
    if (*now == '-') now++;     // 为负号, 跳过
    if (*now >= '0' && *now <= '9') {

        /* 防止前导0, 比如 `-01`, `001`, `00` */
        if (*now == '0' &&     
           (*(now + 1) >= '0' && *(now + 1) <= '9')) {
            free(start);
            return NULL;
        }

    } else {
        /* 防止 `-` 或者无数字 */
        free(start);
        return NULL;
    }

    
    char* end = NULL;
    long long valLL_ = strtoll(start, &end, 10);
    statusTmp_.now += end - start;
    free(start);

    flJson* ret = flJsonLL_New(valLL_);
    if (ret == NULL) {
        return NULL;
    }
    
    /* 解析成功, 更新状态并返回 */
    status_->now = statusTmp_.now;
    return ret;
}

/* 解析浮点数, 异常返回NULL */
static flJson* flJsonDouble_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;

    if (statusTmp_.now + 1 > statusTmp_.tail) return NULL;


    /* 由于strtod函数没法根据len来解析数字, 故创建小型缓冲区 */
    const char* tmp = statusTmp_.now;
    while (tmp < statusTmp_.tail && 
          ((*tmp >= '0' && *tmp <= '9') || *tmp == '-' || *tmp == '+' || *tmp == 'e' || *tmp == 'E' || *tmp == '.')) {
            tmp++;
    }
    char* start = (char*)malloc(tmp - statusTmp_.now + 1);
    if (start == NULL) {
        return NULL;
    }
    memcpy(start, statusTmp_.now, tmp - statusTmp_.now);
    start[tmp - statusTmp_.now] = '\0';

    
    const char* now = start;

    /* 前导判断 */
    if (*now == '-') now++;     // 为负号, 跳过
    if (*now >= '0' && *now <= '9') {

        /* 防止前导0, 比如 `-01`, `001`, `00` */
        if (*now == '0' && 
            *(now + 1) >= '0' && *(now + 1) <= '9') {
            free(start);
            return NULL;
        }

    } else {
        /* 防止 `-` 或者无数字 */
        free(start);
        return NULL;
    }

    /* 判断.后面必须是数字以及e/E后哦吗必须有至少一个数字(可以有+-) */
    while ((*now >= '0' && *now <= '9') || *now == '-' || *now == '+' || *now == 'e' || *now == 'E' || *now == '.') {
        if (*now == '.') {
            now++;
            /* 小数点后面必须要有数 */
            if (!(*now >= '0' && *now <= '9')) {
                free(start);
                return NULL;
            }

        } else if (*now == 'e' || *now == 'E') {
            now++;
            if (*now == '+' || *now == '-') now++;
            
            /* e/E(+-)后面必须要有数 */
            if (!(*now >= '0' && *now <= '9')) {
                free(start);
                return NULL;
            }
        }
        now++;
    }



    char* end = NULL;
    double valDouble_ = strtod(start, &end);
    statusTmp_.now += end - start;
    free(start);


    flJson* ret = flJsonDouble_New(valDouble_);
    if (ret == NULL) {
        return NULL;
    }
    
    /* 解析成功, 更新状态并返回 */
    status_->now = statusTmp_.now;
    return ret;
}


/* 解析字符串, 返回flJson, 异常返回NULL */
static flJson* flJsonString_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);  

    strStatus_ statusTmp_ = *status_;
    
    if (statusTmp_.now + 1 > statusTmp_.tail) return NULL;
    
    char* str = parseStr_(&statusTmp_, depth);

    if (str == NULL) {
        return NULL;
    }
    
    flJson* ret = flJsonString_New(str);
    if (ret == NULL) {
        free(str);
        return NULL;
    }

    
    /* 解析成功, 更新状态并返回 */
    free(str);
    status_->now = statusTmp_.now;
    return ret;
}

/* 解析null, 异常返回NULL */
static flJson* flJsonNull_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;

    if (statusTmp_.now + 4 > statusTmp_.tail) return NULL;

    int cmpRes = strncmp(statusTmp_.now, "null", 4);

    if (cmpRes != 0) {
        return NULL;
    }
    statusTmp_.now += 4;

    flJson* ret = flJsonNull_New();
    if (ret == NULL) {
        return NULL;
    }

    /* 解析成功, 更新状态并返回 */
    status_->now = statusTmp_.now;
    return ret;    
}

/* 解析true, 异常返回NULL */
static flJson* flJsonBoolTrue_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;

    if (statusTmp_.now + 4 > statusTmp_.tail) return NULL;

    int cmpRes = strncmp(statusTmp_.now, "true", 4);

    if (cmpRes != 0) {
        return NULL;
    }
    statusTmp_.now += 4;

    flJson* ret = flJsonBool_New(true);
    if (ret == NULL) {
        return NULL;
    }

    /* 解析成功, 更新状态并返回 */
    status_->now = statusTmp_.now;
    return ret;  
}

/* 解析false, 异常返回NULL */
static flJson* flJsonBoolFalse_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;

    if (statusTmp_.now + 5 > statusTmp_.tail) return NULL;

    int cmpRes = strncmp(statusTmp_.now, "false", 5);

    if (cmpRes != 0) {
        return NULL;
    }
    statusTmp_.now += 5;

    flJson* ret = flJsonBool_New(false);
    if (ret == NULL) {
        return NULL;
    }

    /* 解析成功, 更新状态并返回 */
    status_->now = statusTmp_.now;
    return ret;  
}

/* 解析Object, 异常返回NULL */
static flJson* flJsonObject_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;

    if (statusTmp_.now + 2 > statusTmp_.tail) return NULL;

    if (*statusTmp_.now != '{') return NULL;
    statusTmp_.now++;

    flJson* obj = flJsonObject_New();
    if (obj == NULL) {
        return NULL;
    }

    ignoreSpace_(&statusTmp_);

    if (statusTmp_.now + 1 > statusTmp_.tail) {
        flJson_UnRef(obj);
        return NULL;
    }
    if (*statusTmp_.now == '}') {
        /* 空object */
        statusTmp_.now++;
        status_->now = statusTmp_.now;
        return obj;
    }

    bool flagError = false;
    do {
        /* 解析key */
        char* key = parseStr_(&statusTmp_, depth);
        if (key == NULL) {
            flagError = true;
            break;
        }

        ignoreSpace_(&statusTmp_);

        if (statusTmp_.now + 1 > statusTmp_.tail) {
            free(key);
            flJson_UnRef(obj);
            return NULL;
        }
        if(*statusTmp_.now != ':') {
            free(key);
            flagError = true;
            break;
        }
        statusTmp_.now++;

        /* 解析json */
        flJson* j = flJson_Parse_(&statusTmp_, depth);

        if (j == NULL) {
            flagError = true;
            free(key);
            break;
        }

        /* 插入 */
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
        if (statusTmp_.now + 1 > statusTmp_.tail) {
            flJson_UnRef(obj);
            return NULL;
        }
    } while (*statusTmp_.now == ',' && statusTmp_.now++);   // 如果是逗号, 则跳过, 如果不是, 那个statusTmp_.now是不会加的


    if (flagError || *statusTmp_.now != '}') {
        flJson_UnRef(obj);
        return NULL;
    } else {
        statusTmp_.now++;

        /* 解析成功, 更新状态并返回 */
        status_->now = statusTmp_.now;
        return obj;
    }
}

/* 解析Array, 异常返回NULL */
static flJson* flJsonArray_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    strStatus_ statusTmp_ = *status_;

    if (statusTmp_.now + 2 > statusTmp_.tail) return NULL;

    if (*statusTmp_.now != '[') return NULL;
    statusTmp_.now++;

    flJson* arr = flJsonArray_New();
    if (arr == NULL) {
        return NULL;
    }

    ignoreSpace_(&statusTmp_);

    if (statusTmp_.now + 1 > statusTmp_.tail) {
        flJson_UnRef(arr);
        return NULL;
    }
    if (*statusTmp_.now == ']') {
        /* 空object */
        statusTmp_.now++;
        status_->now = statusTmp_.now;
        return arr;
    }

    bool flagError = false;
    do {
        /* 解析json */
        flJson* j = flJson_Parse_(&statusTmp_, depth);
        if (j == NULL) {
            flagError = true;
            break;
        }

        /* 插入 */
        if (flJsonArray_Add(arr, j, flJsonArray_Size(arr)) == flRet_Suc)  {
            flJson_UnRef(j);
        } else {
            flJson_UnRef(j);
            flagError = true;
            break;
        }

        ignoreSpace_(&statusTmp_);
        
        if (statusTmp_.now + 1 > statusTmp_.tail) {
            flJson_UnRef(arr);
            return NULL;
        }
    } while (*statusTmp_.now == ',' && statusTmp_.now++);   // 如果是逗号, 则跳过, 如果不是, 那个statusTmp_.now是不会加的

    if (flagError || *statusTmp_.now != ']') {
        flJson_UnRef(arr);
        return NULL;
    } else {
        statusTmp_.now++;

        /* 解析成功, 更新状态并返回 */
        status_->now = statusTmp_.now;
        return arr;
    }
}

/* 解析Json, 异常返回NULL */
static flJson* flJson_Parse_(strStatus_* status_, int depth) {
    if (isTooDeep(&depth)) {
        return NULL;
    }
    ignoreSpace_(status_);

    /* 此处为中转站, 无需创建临时状态 */

    if (status_->now + 1 > status_->tail) return NULL;

    char headCh = *status_->now;

    if ((headCh >= '0' && headCh <= '9') || headCh == '-') {
        
        /* 判断是否位浮点数 */
        bool doubleFlag = false;
        const char* tmp = status_->now;
        while (tmp < status_->tail && 
              ((*tmp >= '0' && *tmp <= '9') || *tmp == '-' || *tmp == '+' || *tmp == 'e' || *tmp == 'E' || *tmp == '.')) {

                if (*tmp == 'e' || *tmp == 'E' || *tmp == '.') {
                    doubleFlag = true;
                    break;
                }
                tmp++;
        }

        if (doubleFlag) {
            return flJsonDouble_Parse_(status_, depth);
        } else {
            return flJsonLL_Parse_(status_, depth);
        }
        
    } else if (headCh == 'n') {
        /* Null */
        return flJsonNull_Parse_(status_, depth);
    } else if (headCh == 't') {
        /* True */
        return flJsonBoolTrue_Parse_(status_, depth);
    } else if (headCh == 'f'){
        /* False */
        return flJsonBoolFalse_Parse_(status_, depth);
    } else if (headCh == '\"') {
        /* String */
        return flJsonString_Parse_(status_, depth);
    } else if (headCh == '{') {
        /* Object */
        return flJsonObject_Parse_(status_, depth);
    } else if (headCh == '[') {
        /* Array */
        return flJsonArray_Parse_(status_, depth);
    } else {
        /* Error */
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
        .tail = str + strlen(str),
        .now = str
    };
    flJson* ret = flJson_Parse_(&allStatus, 1);

    ignoreSpace_(&allStatus);

    if (ret == NULL) {
        return NULL;
    } else if (allStatus.now < allStatus.tail){
        flJson_UnRef(ret);
        return NULL;
    } else {
        return ret;
    }
}

/**
 * 解析指定长度的字符串为Json
 * 
 * @return - 解析出错返回NULL
 */
flJson* flJson_ParseWithLength(const char* str, size_t len) {
    strStatus_ allStatus = {
        .tail = str + len,
        .now = str
    };
    flJson* ret = flJson_Parse_(&allStatus, 1);

    ignoreSpace_(&allStatus);

    if (ret == NULL) {
        return NULL;
    } else if (allStatus.now < allStatus.tail){
        /* 解析完成后应该到达尾指针, 如果不是, 说明错误 */
        flJson_UnRef(ret);
        return NULL;
    } else {
        return ret;
    }
}
