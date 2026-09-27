/*
 * Copyright (c) 2026 Flmpx
 * Licensed under MIT (see LICENSE).
 */


#define _XOPEN_SOURCE 700

#include "../include/flJson.h"
#include <string.h>

/* 引入 hm_map 和 hm_arr */
#include <hm_map.h>
#include <hm_arr.h>


/* 不同类型Json的创建 */

/**
 * 创建LL类型的Json
 * 
 * @return - 如果创建失败, 返回NULL
 */
flJson* flJsonLL_New(long long ll) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }

    ret->type_ = flJsonTypeLL;
    ret->refCount_ = 1;
    ret->valLL_ = ll;

    return ret;
}

/**
 * 创建Double类型的Json
 * 
 * @return - 如果创建失败, 返回NULL
 */
flJson* flJsonDouble_New(double d) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeDouble;
    ret->refCount_ = 1;
    ret->valDouble_ = d;

    return ret;
}

/**
 * 创建Bool类型的Json
 * 
 * @return - 如果创建失败, 返回NULL
 */
flJson* flJsonBool_New(bool b) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeBool;
    ret->refCount_ = 1;
    ret->valBool_ = b;

    return ret;
}

/**
 * 创建Null类型的Json
 * 
 * @return - 如果创建失败, 返回NULL
 */
flJson* flJsonNull_New() {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeNull;
    ret->refCount_ = 1;

    return ret;
}

/**
 * 创建Object类型的Json
 * 
 * @return - 如果创建失败, 返回NULL
 */
flJson* flJsonObject_New() {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeObject;
    ret->refCount_ = 1;

    ret->valObject_.cap_ = 0;
    ret->valObject_.entrys_ = NULL;
    ret->valObject_.status_ = NULL;
    ret->valObject_.size_ = 0;

    return ret;
    
}

/**
 * 创建Array类型的Json
 * 
 * @return - 如果创建失败, 返回NULL
 */
flJson* flJsonArray_New() {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeArray;
    ret->refCount_ = 1;

    ret->valArray_.array_ = NULL;
    ret->valArray_.cap_ = 0;
    ret->valArray_.size_ = 0;
    
    return ret;
    
}

/**
 * 创建String类型的Json
 * 
 * @note - 字符串会深拷贝
 * 
 * @return - 如果创建失败, 返回NULL
 */
flJson* flJsonString_New(const char* s) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    /* 复制字符串 */
    char* new_s = strdup(s);
    if (new_s == NULL) {
        free(ret);
        return NULL;
    }
    
    ret->type_ = flJsonTypeString;
    ret->refCount_ = 1;
    ret->valString_ = new_s;
    
    return ret;
}

/* Json类型的判断 */

/**
 * 检测Json的类型
 * 
 * @return - 如果类型不匹配, 返回false
 */
bool flJson_CheckType(flJson* j, flJsonType type) {
    return j->type_ & type;
}




/* JsonLL的操作 */

/**
 * 获取LL型Json的内部数据
 * 
 * @return - 如果类型错误返回NULL
 */
long long* flJsonLL_Get(flJson* jll) {
    if(!flJson_CheckType(jll, flJsonTypeLL)) {
        return NULL;
    }

    return &(jll->valLL_);
}

/* JsonDouble的操作 */

/**
 * 获取Double型Json的内部数据
 * 
 * @return - 如果类型错误返回NULL
 */
double* flJsonDouble_Get(flJson* jd) {
    if(!flJson_CheckType(jd, flJsonTypeDouble)) {
        return NULL;
    }

    return &(jd->valDouble_);
}

/* JsonString的操作 */

/**
 * 获取String型Json的内部数据
 * 
 * @return - 如果类型错误返回NULL
 */
char* flJsonString_Get(flJson* js) {
    if(!flJson_CheckType(js, flJsonTypeString)) {
        return NULL;
    }

    return js->valString_;
}

/* JsonBool的操作 */

/**
 * 获取Bool型Json的内部数据
 * 
 * @return - 如果类型错误返回NULL
 */
bool* flJsonBool_Get(flJson* jb) {
    if(!flJson_CheckType(jb, flJsonTypeBool)) {
        return NULL;
    }

    return &(jb->valBool_);
}


/**
 * 用于hm_map <-> flObject, hm_arr <-> flArray, hm_map_iter <-> flObjectIter 之间的内容转化
 */

/* 对字符串进行hash */
static inline size_t hashString_(const char* str) {
    size_t res = 5381;
    int c;
    while (c = *str++) {
        res = ((res << 5) + res) + c;
    }
    return res;
}

/* 将Object的内部信息 --> hm_map */
static inline void flObject__TO__hm_map_(flJson* jo, hm_map* m) {
    m->buckets = (hm_map_entry*)jo->valObject_.entrys_;
    m->buckets_status = (hm_map_entry_status*)jo->valObject_.status_;
    m->cmp_key = (hm_cmp)strcmp;
    m->free_key = (hm_free)free;
    m->free_val = (hm_free)flJson_UnRef;
    m->hash_key = (hm_hash)hashString_;
    m->len = jo->valObject_.cap_;
    m->size = jo->valObject_.size_;
}

/* 将hm_map的内部信息 --> Object */
static inline void hm_map__TO__flObject_(hm_map* m, flJson* jo) {
    jo->valObject_.cap_ = m->len;
    jo->valObject_.size_ = m->size;
    jo->valObject_.entrys_ = (void*)m->buckets;
    jo->valObject_.status_ = (int*)m->buckets_status;
}

/* 将Array的内部信息 --> hm_arr */
static inline void flArray__TO__hm_arr_(flJson* ja, hm_arr* a) {
    a->capacity = ja->valArray_.cap_;
    a->dynamic_grow = true;
    a->free_val = (hm_free)flJson_UnRef;
    a->size = ja->valArray_.size_;
    a->vals = (void**)ja->valArray_.array_;
}

/* 将hm_arr的内部信息 --> Array */
static inline void hm_arr__TO__flArray_(hm_arr* a, flJson* ja) {
    ja->valArray_.array_ = (flJson**)a->vals;
    ja->valArray_.cap_ = a->capacity;
    ja->valArray_.size_ = a->size;
}



/* 将ObjectIter中的内部信息 --> hm_arr_iter */
static inline void flObjectIter__TO__hm_map_iter_(flJsonObjectIter* joi, hm_map_iter* mi) {
    mi->buckets = joi->entrys_;
    mi->buckets_status = (hm_map_entry_status*)joi->status_;
    mi->index = joi->idx_;
    mi->len = joi->cap_;
}

/* 将hm_arr_iter中的内部信息 --> ObjectIter */
static inline void hm_map_iter__TO__flObjectIter_(hm_map_iter* mi, flJsonObjectIter* joi) {
    joi->entrys_ = (void*)mi->buckets;
    joi->status_ = (int*)mi->buckets_status;
    joi->idx_ = mi->index;
    joi->cap_ = mi->len;
}



/* JsonArray的操作 */

/**
 * 获取Array类型Json的大小
 * 
 * @return - 如果类型不对, 返回0
 */
size_t flJsonArray_Size(flJson* ja) {
    if (!flJson_CheckType(ja, flJsonTypeArray)) {
        return 0;
    }

    return ja->valArray_.size_;
}

/**
 * 在数组指定位置指定位置插入flJson
 * 
 * @note - 如果待插入的位置大于数组的大小, 函数将自动校正为插入至尾部
 * 
 * @return - 如果插入成功, 返回flRet_Suc  
 * @return - 如果插入失败, 返回flRet_Error  
 * @return - 如果类型不对, 返回flRet_Warn  
 * 
 * @warning - 不可以将上级Json插入到下级Json中
 */
flRet flJsonArray_Add(flJson* ja, flJson* j, size_t idx) {
    if (!flJson_CheckType(ja, flJsonTypeArray)) {
        return flRet_Warn;
    }

    size_t s = ja->valArray_.size_;
    /* 自动矫正 */
    idx = idx > s ? s : idx;

    hm_arr arr;
    flArray__TO__hm_arr_(ja, &arr);       // 转化

    hm_arr_ret retCode = hm_arr_insert_index(&arr, j, idx);

    if (retCode != hm_arr_ret_suc) {
        return flRet_Error;
    } else {
        j->refCount_++;
        hm_arr__TO__flArray_(&arr, ja);    // 转化
        return flRet_Suc;
    }
}

/**
 * 删除指定下标的flJson
 * 
 * @return - 如果删除成功, 返回flRet_Suc
 * @return - 如果下标不合法, 返回flRet_None
 * @return - 如果类型不对, 返回flRet_Warn
 */
flRet flJsonArray_Del(flJson* ja, size_t idx) {
    if (!flJson_CheckType(ja, flJsonTypeArray)) {
        return flRet_Warn;
    }

    if (idx > ja->valArray_.size_) {
        return flRet_None;
    }

    hm_arr arr;
    flArray__TO__hm_arr_(ja, &arr);

    hm_arr_del_index(&arr, idx);

    hm_arr__TO__flArray_(&arr, ja);

    return flRet_Suc;
}

/**
 * 获取指定位置的flJson
 * 
 * @note - 不增加返回的Json节点的引用计数
 * 
 * @return - 如果下标不合法, 返回NULL
 * @return - 如果类型不对, 返回NULL
 */
flJson* flJsonArray_Get(flJson* ja, size_t idx) {
    if (!flJson_CheckType(ja, flJsonTypeArray)) {
        return NULL;
    }

    hm_arr arr;
    flArray__TO__hm_arr_(ja, &arr);

    flJson* ret = hm_arr_get(&arr, idx);

    if (ret == NULL) {
        return NULL;
    } else {
        return ret;
    }
}


/**
 * 初始化数组型Json迭代器
 */
void flJsonArrayIter_Init(flJsonArrayIter* jai, flJson* ja) {
    jai->array_ = ja->valArray_.array_;
    jai->size_ = ja->valArray_.size_;
    jai->idx_ = 0;
}


/**
 * 数组迭代器当前指向是否有效
 */
bool flJsonArrayIter_HasCur(flJsonArrayIter* jai) {
    return jai->idx_ < jai->size_;
} 


/**
 * 获取当前数组迭代器所指向的Json
 * 
 * @note - 返回的Json不增加引用次数
 * 
 * @return - 如果当前指向无效, 返回NULL
 */
flJson* flJsonArrayIter_Cur(flJsonArrayIter* jai) {
    if (flJsonArrayIter_HasCur(jai)) {
        return jai->array_[jai->idx_];
    } else {
        return NULL;
    }
}


/**
 * 将数组迭代器指向移动到下一个位置
 */
void flJsonArrayIter_MoveNext(flJsonArrayIter* jai) {
    if (jai->idx_ < jai->size_) {
        (jai->idx_)++;
    }
}




/* JsonObject的操作 */


/**
 * 获取Object类型Json的大小
 * 
 * @return - 如果类型不对, 返回0
 */
size_t flJsonObject_Size(flJson* jo) {
    if (!flJson_CheckType(jo, flJsonTypeObject)) {
        return 0;
    }
    
    return jo->valObject_.size_;
} 

/**
 * 添加条目(key和flJson)到对象中
 * 
 * @note - 如果有重复键, 该函数直接替换掉flJson
 * @note - 如果既是重复键, 同时插入的flJson的地址也和内部一样, 那这个函数等于什么也没做
 * 
 * @return - 如果插入成功, 返回flRet_Suc
 * @return - 如果插入失败, 返回flRet_Error
 * @return - 如果类型不对, 返回flRet_Warn
 * 
 * @warning - 不可以将上级Json插入到下级Json中
 */
flRet flJsonObject_Add(flJson* jo, const char* key, flJson* j) {
    if (!flJson_CheckType(jo, flJsonTypeObject)) {
        return flRet_Warn;
    }

    char* new_s = strdup(key);
    if (new_s == NULL) {
        return flRet_Error;
    }

    hm_map map;
    flObject__TO__hm_map_(jo, &map);

    hm_map_ret retCode = hm_map_insert(&map, new_s, j);

    if (retCode == hm_map_ret_error) {
        free(new_s);
        return flRet_Error;
    } else if (retCode == hm_map_ret_existed) {
        /* 重复键处理 */
        hm_map_entry* tmp = hm_map_get_entry(&map, (void*)key);
        free(new_s);

        if (j != tmp->val) {
            /* 如果插入的不是相同的才更新 */
            flJson_UnRef(tmp->val); // 删掉旧的

            tmp->val = j;

            j->refCount_++;
        }


    } else {
        /* 正常插入 */
        j->refCount_++;
    }
    hm_map__TO__flObject_(&map, jo);
    
    return flRet_Suc;

}

/**
 * 删除对象中的条目(key和flJson)
 * 
 * @return - 如果删除成功, 返回flRet_Suc
 * @return - 如果键不存在, 返回flRet_None
 * @return - 如果类型不对, 返回flRet_Warn
 */
flRet flJsonObject_Del(flJson* jo, const char* key) {
    if (!flJson_CheckType(jo, flJsonTypeObject)) {
        return flRet_Warn;
    }

    hm_map map;
    flObject__TO__hm_map_(jo, &map);

    hm_map_ret retCode = hm_map_del(&map, (void*)key);
    
    if (retCode == hm_map_ret_none) {
        return flRet_None;
    } else {
        return flRet_Suc;
    }

}

/**
 * 获取对象中的指定键对应的flJson
 * 
 * @note - 不增加返回的Json节点的引用计数
 * 
 * @return - 如果键不存在, 返回NULL
 * @return - 如果类型不对, 返回NULL
 */
flJson* flJsonObject_Get(flJson* jo, const char* key) {
    if (!flJson_CheckType(jo, flJsonTypeObject)) {
        return NULL;
    }

    hm_map map;
    flObject__TO__hm_map_(jo, &map);

    flJson* ret = hm_map_get(&map, (void*)key).val;

    if (ret == NULL) {
        return NULL;
    } else {
        return ret;
    }
}


/**
 * 初始化对象的迭代器
 */
void flJsonObjectIter_Init(flJsonObjectIter* joi, flJson* jo) {
    joi->cap_ = jo->valObject_.cap_;
    joi->entrys_ = jo->valObject_.entrys_;
    joi->idx_ = 0;
    joi->status_ = jo->valObject_.status_;
}


/**
 * 对象迭代器当前指向是否有效
 */
bool flJsonObjectIter_HasCur(flJsonObjectIter* joi) {
    /* 由于hashTable不支持随机访问, 所有不能简单的判断当前指向是否有效 */

    hm_map_iter it;
    flObjectIter__TO__hm_map_iter_(joi, &it);
    /* has_next函数会自动往后面移动迭代器指向, 如果连后面(包括当前指向)都没有有效值, 那直接返回false */
    bool ret = hm_map_iter_has_next(&it);
    hm_map_iter__TO__flObjectIter_(&it, joi);

    return ret;
}


/**
 * 获取当前对象迭代器所指向条目的Json
 * 
 * @note - 返回的Json不增加引用次数
 * 
 * @return - 如果当前指向无效, 返回NULL
 */
flJson* flJsonObjectIter_CurVal(flJsonObjectIter* joi) {
    if (flJsonObjectIter_HasCur(joi)) {
        return ((hm_map_entry*)joi->entrys_)[joi->idx_].val;
    } else {
        return NULL;
    }
}

/**
 * 获取当前对象迭代器所指向条目的key
 * 
 * @return - 如果当前指向无效, 返回NULL
 */
const char* flJsonObjectIter_CurKey(flJsonObjectIter* joi) {
    if (flJsonObjectIter_HasCur(joi)) {
        return ((hm_map_entry*)joi->entrys_)[joi->idx_].key;
    } else {
        return NULL;
    }
}

/**
 * 将对象迭代器指向移动到下一个位置
 */
void flJsonObjectIter_MoveNext(flJsonObjectIter* joi) {
    hm_map_iter iter;
    flObjectIter__TO__hm_map_iter_(joi, &iter);
    /* 直接忽略这里的返回值, 只需要它的移动功能 */
    hm_map_iter_next(&iter);
    hm_map_iter__TO__flObjectIter_(&iter, joi);
}




/* Unref解引Json */

/* 释放掉Json中的Array, 等于直接调用hm_arr_free函数 */
static void flArray_Free_(flJson* ja) {

    hm_arr arr;
    flArray__TO__hm_arr_(ja, &arr);

    hm_arr_free(&arr);

}

/* 释放掉Json中的Object, 等于直接调用hm_map_free函数 */
static void flObject_Free_(flJson* jo) {

    hm_map map;
    flObject__TO__hm_map_(jo, &map);

    hm_map_free(&map);
    
}

/**
 * 对Json进行解引用
 */
void flJson_UnRef(flJson* j) {
    if (j == NULL) return;
    if (j->refCount_ == 0) return;

    j->refCount_--;

    if (j->refCount_ == 0) {
        /* 只有string, object, array特殊处理 */
        switch (j->type_) {
            case flJsonTypeString: free(j->valString_);             break;
            case flJsonTypeObject: flObject_Free_(j);                break;
            case flJsonTypeArray:  flArray_Free_(j);                 break;
        }
        free(j);
    }

    
}
