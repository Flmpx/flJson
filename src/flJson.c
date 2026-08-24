#define _XOPEN_SOURCE 700
#include "../include/flJson.h"
#include <string.h>

// 引入 hm_map 和 hm_arr
#include <hm_map.h>
#include <hm_arr.h>


/**
 * 创建Json, 如果创建失败, 返回NULL
 */

flJson* flJson_NewInt(int i) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }

    ret->type_ = flJsonTypeInt;
    ret->refCout_ = 1;
    ret->valInt_ = i;

    return ret;
}

flJson* flJson_NewDouble(double d) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeDouble;
    ret->refCout_ = 1;
    ret->valDouble_ = d;

    return ret;
}

flJson* flJson_NewBool(bool b) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeBool;
    ret->refCout_ = 1;
    ret->valBool_ = b;

    return ret;
}

flJson* flJson_NewNull() {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeNull;
    ret->refCout_ = 1;

    return ret;
}

flJson* flJson_NewObject(flObject* o) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeObject;
    ret->refCout_ = 1;
    ret->valObject_ = o;

    o->refCout_++;

    return ret;
    
}

flJson* flJson_NewArray(flArray* a) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    
    ret->type_ = flJsonTypeArray;
    ret->refCout_ = 1;
    ret->valArray_ = a;

    a->refCout_++;
    
    return ret;
    
}

flJson* flJson_NewString(const char* s) {
    flJson* ret = (flJson*)malloc(sizeof(flJson));
    if (ret == NULL) {
        return NULL;
    }
    // 复制字符串
    char* new_s = strdup(s);
    if (new_s == NULL) {
        free(ret);
        return NULL;
    }
    
    ret->type_ = flJsonTypeString;
    ret->refCout_ = 1;
    ret->valString_ = new_s;
    
    return ret;
}


/**
 * 检测Json的类型, 如果不是该类型, 返回false
 */

bool flJson_CheckType(flJson* j, flJsonType type) {
    return j->type_ & type;
}



/**
 * 获取Json内部的数据
 * 可以通过返回的指针进行修改数据, 返回flArray* , flObject* 的会增加对应的容器内部的引用计数
 * 如果类型不对返回空指针
 */

int* flJson_GetInt(flJson* j) {
    if(!flJson_CheckType(j, flJsonTypeInt)) {
        return NULL;
    }

    return &(j->valInt_);
}

double* flJson_GetDouble(flJson* j) {
    if(!flJson_CheckType(j, flJsonTypeDouble)) {
        return NULL;
    }

    return &(j->valDouble_);
}

char* flJson_GetString(flJson* j) {
    if(!flJson_CheckType(j, flJsonTypeString)) {
        return NULL;
    }

    return j->valString_;
}

flObject* flJson_GetObject(flJson* j) {
    if(!flJson_CheckType(j, flJsonTypeObject)) {
        return NULL;
    }

    j->refCout_++;

    return j->valObject_;
}

flArray* flJson_GetArray(flJson* j) {
    if(!flJson_CheckType(j, flJsonTypeArray)) {
        return NULL;
    }
    
    j->refCout_++;
    
    return j->valArray_;
}

bool* flJson_GetBool(flJson* j) {
    if(!flJson_CheckType(j, flJsonTypeBool)) {
        return NULL;
    }

    return &(j->valBool_);
}


/**
 * 用于hm_map <-> flObject, hm_arr <-> flArray 之间的内容转化
 */

static size_t hash_string(const char* str) {
    size_t res = 5381;
    int c;
    while (c = *str++) {
        res = ((res << 5) + res) + c;
    }
    return res;
}

static void flObject__TO__hm_map(flObject* o, hm_map* m) {
    m->buckets = (hm_map_entry*)o->entrys_;
    m->buckets_status = (hm_map_entry_status*)o->status_;
    m->cmp_key = (hm_cmp)strcmp;
    m->free_key = (hm_free)free;
    m->free_val = (hm_free)flJson_UnRef;
    m->hash_key = (hm_hash)hash_string;
    m->len = o->cap_;
    m->size = o->size_;
}

static void hm_map__TO__flObject(hm_map* m, flObject* o) {
    o->cap_ = m->len;
    o->size_ = m->size;
    o->entrys_ = (struct flObjectEntry*)m->buckets;
    o->status_ = (enum flObjectEntryStatus_*)m->buckets_status;
}

static void flArray__TO__hm_arr(flArray* fa, hm_arr* ha) {
    ha->capacity = fa->cap_;
    ha->dynamic_grow = true;
    ha->free_val = (hm_free)flJson_UnRef;
    ha->size = fa->size_;
    ha->vals = (void**)fa->array_;
}

static void hm_arr__TO__flArray(hm_arr* ha, flArray* fa) {
    fa->array_ = (flJson**)ha->vals;
    fa->cap_ = ha->capacity;
    fa->size_ = ha->size;
}



/**
 * Array的相关操作
 */


/**
 * 创建一个数组
 * 
 * @return 如果创建失败返回NULL
 */
flArray* flArray_New() {
    flArray* ret = (flArray*)malloc(sizeof(flArray));
    if (ret == NULL) {
        return NULL;
    }

    ret->array_ = NULL;
    ret->cap_ = 0;
    ret->refCout_ = 1;
    ret->size_ = 0;

    return ret;
}

/**
 * 在数组指定位置指定位置插入flJson
 * 
 * @note 如果待插入的位置大于数组的大小, 函数将自动校正为插入至尾部
 * 
 * @return 如果插入失败, 返回flRet_Error, 成功则flRet_Suc
 */
flRet flArray_Add(flArray* a, flJson* j, size_t idx) {
    size_t s = a->size_;
    // 自动矫正
    idx = idx > s ? s : idx;

    hm_arr arr;
    flArray__TO__hm_arr(a, &arr);       // 转化

    hm_arr_ret retCode = hm_arr_insert_index(&arr, j, idx);

    if (retCode != hm_arr_ret_suc) {
        return flRet_Error;
    } else {
        j->refCout_++;
        hm_arr__TO__flArray(&arr, a);    // 转化
    }
}

/**
 * 删除指定下标的flJson
 * 
 * @return 如果下标不合法, 那返回flRet_None, 否则flRet_Suc
 */
flRet flArray_Del(flArray* a, size_t idx) {
    if (idx > a->size_) {
        return flRet_None;
    }
    hm_arr arr;
    flArray__TO__hm_arr(a, &arr);

    hm_arr_del_index(&arr, idx);

    hm_arr__TO__flArray(&arr, a);

    return flRet_Suc;
}

/**
 * 获取指定位置的flJson
 * 
 * @note 返回后flJson的引用次数加一, 使用完后使用对应的UnRef函数解引
 * 
 * @return 如果下标不合法, 返回NULL
 */
flJson* flArray_Get(flArray* a, size_t idx) {
    hm_arr arr;
    flArray__TO__hm_arr(a, &arr);

    flJson* ret = hm_arr_get(&arr, idx);

    if (ret == NULL) {
        return NULL;
    } else {
        ret->refCout_++;
        return ret;
    }
}




/**
 * Object的相关操作
 */


/**
 * 创建一个对象
 * 
 * @return 如果创建失败返回NULL
 */
flObject* flObject_New() {
    flObject* ret = (flObject*)malloc(sizeof(flObject));
    if (ret == NULL) {
        return NULL;
    }

    ret->cap_ = 0;
    ret->entrys_ = NULL;
    ret->refCout_ = 1;
    ret->size_ = 0;
    ret->status_ = NULL;

    return ret;
}

/**
 * 添加条目到对象中
 * 
 * @note 如果有重复键, 该函数直接替换掉flJson
 * @note 如果既是重复键, 同时插入的flJson的地址也和内部一样, 那这个函数等于什么也没做
 * 
 * @return 如果插入成功, 返回flRet_Suc, 如果插入失败, 返回
 */
flRet flObject_Add(flObject* o, const char* key, flJson* j) {

    char* new_s = strdup(key);
    if (new_s == NULL) {
        return flRet_Error;
    }

    hm_map map;
    flObject__TO__hm_map(o, &map);

    hm_map_ret retCode = hm_map_insert(&map, new_s, j);

    if (retCode == hm_map_ret_error) {
        free(new_s);
        return flRet_Error;
    } else if (retCode == hm_map_ret_existed) {
        // 重复键处理
        hm_map_entry* tmp = hm_map_get_entry(&map, (void*)key);
        free(new_s);

        if (j != tmp->val) {
            // 如果插入的不是相同的才更新
            flJson_UnRef(tmp->val); // 删掉旧的

            tmp->val = j;

            j->refCout_++;
        }


    } else {
        // 正常插入
        j->refCout_++;
    }
    hm_map__TO__flObject(&map, o);
    
    return flRet_Suc;

}

/**
 * 删除对象中的条目
 * 
 * @return 如果键不存在, 返回flRet_None, 否则flRet_Suc
 */
flRet flObject_Del(flObject* o, const char* key) {
    hm_map map;
    flObject__TO__hm_map(o, &map);

    hm_map_ret retCode = hm_map_del(&map, (void*)key);
    
    if (retCode == hm_map_ret_none) {
        return flRet_None;
    } else {
        return flRet_Suc;
    }

}

/**
 * 获取对象中的指定键对应的Json
 * 
 * @note 返回后flJson的引用次数加一, 使用完后使用对应的对应的UnRef函数解引
 * 
 * @return 如果键不存在, 返回NULL
 */
flJson* flObject_Get(flObject* o, const char* key) {
    hm_map map;
    flObject__TO__hm_map(o, &map);

    flJson* ret = hm_map_get(&map, (void*)key).val;

    if (ret == NULL) {
        return NULL;
    } else {
        ret->refCout_++;
        return ret;
    }
}



/**
 * Unref 解除引用
 */

void flJson_UnRef(flJson* j) {
    if (j->refCout_ == 0) return;

    j->refCout_--;

    if (j->refCout_ == 0) {
        // 只有string, object, array特殊处理
        switch (j->type_) {
            case flJsonTypeString: free(j->valString_);             break;
            case flJsonTypeObject: flObject_UnRef(j->valObject_);   break;
            case flJsonTypeArray:  flArray_UnRef(j->valArray_);     break;
        }
        free(j);
    }
    
}
void flArray_UnRef(flArray* a) {
    if (a->refCout_ == 0) return;

    a->refCout_--;

    if (a->refCout_ == 0) {
        hm_arr arr;
        flArray__TO__hm_arr(a, &arr);

        hm_arr_free(&arr);

        free(a);
    }
    
}
void flObject_UnRef(flObject* o) {
    if (o->refCout_ == 0) return;

    o->refCout_--;
    
    if (o->refCout_ == 0) {
        hm_map map;
        flObject__TO__hm_map(o, &map);

        hm_map_free(&map);

        free(o);
    }
    
}