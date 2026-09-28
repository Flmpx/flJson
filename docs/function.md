# 函数详述


## 不同类型的Json的相关函数

Json类型内部具备引用计数, 当调用New函数获取一个新的Json节点时(解析字符串相当于获取一个新的Json节点)或者将一个Json节点插入到其他容器(数组或者对象)中时会增加引用计数, 其他返回flJson*的函数不会增加引用计数, **故每一个New函数都该有一个对应的UnRef函数**  
  
Json中包含字符串类型的(String类型Json, Object中的key)均不支持\0(\u0000)存入, 即不支持存入字符串长度  

下面的函数在Debug模式的构建下, 如果传入空指针, 会断言

### `NULL` 类型的Json

> **创建**

```c
/**
 * 创建Null类型的Json
 * 
 * @return 如果创建失败, 返回NULL
 */
flJson* flJsonNull_New();
```
  
  
### `Long Long` 类型的Json

> **创建**

```c
/**
 * 创建LL类型的Json
 * 
 * @return 如果创建失败, 返回NULL
 */
flJson* flJsonLL_New(long long ll);
```

通过传入一个long long类型的整数, 将其嵌入一个Json中并返回

> **获取**


```c
/**
 * 获取LL型Json的内部数据
 * 
 * @return 如果类型错误返回NULL
 */
long long* flJsonLL_Get(flJson* jll);
```



### `Double` 类型的Json

> **创建**

```c
/**
 * 创建Double类型的Json
 * 
 * @return 如果创建失败, 返回NULL
 */
flJson* flJsonDouble_New(double d);
```

通过传入一个double类型的浮点数, 将其嵌入一个Json中并返回


> **获取**

```c
/**
 * 获取Double型Json的内部数据
 * 
 * @return 如果类型错误返回NULL
 */
double* flJsonDouble_Get(flJson* jd);
```

### `Bool` 类型的Json

> **创建**

```c
/**
 * 创建Bool类型的Json
 * 
 * @return 如果创建失败, 返回NULL
 */
flJson* flJsonBool_New(bool b);
```

通过传入一个bool类型的变量, 将其嵌入一个Json中并返回, 可选的是`true`和`false`

> **获取**

```c
/**
 * 获取Bool型Json的内部数据
 * 
 * @return 如果类型错误返回NULL
 */
bool* flJsonBool_Get(flJson* jb);
```


### `String` 类型的Json

> **创建**

```c
/**
 * 创建String类型的Json
 * 
 * @note 字符串会深拷贝
 * 
 * @return 如果创建失败, 返回NULL
 */
flJson* flJsonString_New(const char* s);
```

传入一个字符串指针, 将传入的字符串**拷贝**后将指针嵌入Json中并返回, **传入的字符串指针不可以为NULL**

> **获取**

```c
/**
 * 获取String型Json的内部数据
 * 
 * @return 如果类型错误返回NULL
 */
char* flJsonString_Get(flJson* js);
```

### `Array` 类型的Json

> **创建**

```c
/**
 * 创建Array类型的Json
 * 
 * @return 如果创建失败, 返回NULL
 */
flJson* flJsonArray_New();
```

> **获取数组大小**

```c
/**
 * 获取Array类型Json的大小
 * 
 * @return 如果类型不对, 返回0
 */
size_t flJsonArray_Size(flJson* ja);
```

> **插入**

```c
/**
 * 在数组指定位置指定位置插入flJson
 * 
 * @note 如果待插入的位置大于数组的大小, 函数将自动校正为插入至尾部
 * 
 * @return 如果插入成功, 返回flRet_Suc  
 * @return 如果插入失败, 返回flRet_Error  
 * @return 如果类型不对, 返回flRet_Warn  
 * 
 * @warning 不可以将上级Json插入到下级Json中
 */
flRet flJsonArray_Add(flJson* ja, flJson* j, size_t idx);
```

该函数是将Json插入到数组中指定下标的位置, 同时将大于等于原下标的元素全部向后移动一个位置

> **获取**

```c
/**
 * 获取指定位置的flJson
 * 
 * @note 不增加返回的Json节点的引用计数
 * 
 * @return 如果下标不合法, 返回NULL
 * @return 如果类型不对, 返回NULL
 */
flJson* flJsonArray_Get(flJson* ja, size_t idx);
```

> **删除**

```c
/**
 * 删除指定下标的flJson
 * 
 * @return 如果删除成功, 返回flRet_Suc
 * @return 如果下标不合法, 返回flRet_None
 * @return 如果类型不对, 返回flRet_Warn
 */
flRet flJsonArray_Del(flJson* ja, size_t idx);
```

该函数将当前下标的Json解引后, 将大于这个下标的Json往前移一个位置

> **迭代**

```c
/**
 * 初始化数组型Json迭代器
 */
void flJsonArrayIter_Init(flJsonArrayIter* jai, flJson* ja);

/**
 * 数组迭代器当前指向是否有效
 */
bool flJsonArrayIter_HasCur(flJsonArrayIter* jai);

/**
 * 获取当前数组迭代器所指向的Json
 * 
 * @note 返回的Json不增加引用次数
 * 
 * @return 如果当前指向无效, 返回NULL
 */
flJson* flJsonArrayIter_Cur(flJsonArrayIter* jai);

/**
 * 将数组迭代器指向移动到下一个位置
 */
void flJsonArrayIter_MoveNext(flJsonArrayIter* jai);
```

以上的函数最好一起通过一个循环使用, 而不是分开使用  



### `Object` 类型的Json

> **创建**

```c
/**
 * 创建Object类型的Json
 * 
 * @return 如果创建失败, 返回NULL
 */
flJson* flJsonObject_New();
```

> **获取对象大小**

```c
/**
 * 获取Object类型Json的大小
 * 
 * @return 如果类型不对, 返回0
 */
size_t flJsonObject_Size(flJson* jo);
```


> **插入**

```c
/**
 * 添加条目(key和flJson)到对象中
 * 
 * @note 如果有重复键, 该函数直接替换掉flJson
 * @note 如果既是重复键, 同时插入的flJson的地址也和内部一样, 那这个函数等于什么也没做
 * 
 * @return 如果插入成功, 返回flRet_Suc
 * @return 如果插入失败, 返回flRet_Error
 * @return 如果类型不对, 返回flRet_Warn
 * 
 * @warning 不可以将上级Json插入到下级Json中
 */
flRet flJsonObject_Add(flJson* jo, const char* key, flJson* j);
```

通过该函数可以向对象中插入一个新的键值对  
值得注意的一点是: 如果出现重复键, 则该函数会直接使用新插入的Json**覆盖并解引**掉原来的Json, 但是原来的key不会改变, 毕竟内容是一样的

> **获取**

```c
/**
 * 获取对象中的指定键对应的flJson
 * 
 * @note 不增加返回的Json节点的引用计数
 * 
 * @return 如果键不存在, 返回NULL
 * @return 如果类型不对, 返回NULL
 */
flJson* flJsonObject_Get(flJson* jo, const char* key);
```


> **删除**

```c
/**
 * 删除对象中的条目(key和flJson)
 * 
 * @return 如果删除成功, 返回flRet_Suc
 * @return 如果键不存在, 返回flRet_None
 * @return 如果类型不对, 返回flRet_Warn
 */
flRet flJsonObject_Del(flJson* jo, const char* key);
```

> **迭代**

```c
/**
 * 初始化对象的迭代器
 */
void flJsonObjectIter_Init(flJsonObjectIter* joi, flJson* jo);

/**
 * 对象迭代器当前指向是否有效
 */
bool flJsonObjectIter_HasCur(flJsonObjectIter* joi);

/**
 * 获取当前对象迭代器所指向条目的key
 * 
 * @return 如果当前指向无效, 返回NULL
 */
const char* flJsonObjectIter_CurKey(flJsonObjectIter* joi);

/**
 * 获取当前对象迭代器所指向条目的Json
 * 
 * @note 返回的Json不增加引用次数
 * 
 * @return 如果当前指向无效, 返回NULL
 */
flJson* flJsonObjectIter_CurVal(flJsonObjectIter* joi);

/**
 * 将对象迭代器指向移动到下一个位置
 */
void flJsonObjectIter_MoveNext(flJsonObjectIter* joi);
```

以上的函数最好一起通过一个循环使用, 而不是分开使用  



### 总结

>  [!Note]
>  `New` 用于创建Json节点 
>  `Get` 用于获取内部数据, 同时由于返回的是指针类型, 可以对数据进行修改, 如果修改, 引用它的所有容器得到的数据都是修改之后的
>  每次创建或者获得一个**新**的Json节点时, 引用计数++

## `UnRef` 解引函数

```c
/**
 * 对Json进行解引用
 */
void flJson_UnRef(flJson* j);
```

对该Json的内部引用计数减少一次, 如果引用次数变为0, 自动进行释放


## 字符串 --> Json树

  
- 解析函数有递归深度限制, 不可以解析嵌套过深的Json文本, 否则返回NULL
- 文本中的字符串类型(字符串型Json或者Object中的key) 不支持 `\u0000`, 如果有, 则解析失败, 返回NULL
- 其他和Json标准相同

### 解析字符串

```c
/**
 * 解析字符串为Json
 * 
 * @return 解析出错返回NULL
 */
flJson* flJson_Parse(const char* str);
```

解析是通过字符串的结束标志`\0`来表示需要解析的文本长度

### 带长度解析字符串

```c
/**
 * 解析指定长度的字符串为Json
 * 
 * @return 解析出错返回NULL
 */
flJson* flJson_ParseWithLength(const char* str, size_t len);
```

指定需要解析的Json文本长度  


## Json树 --> 字符串

- 转字符串函数具有递归深度限制, 不可以输出嵌套过深的Json树, 否则返回NULL  

### 输出字符串

```c
/**
 * 输出Json为字符串
 * 
 * @return 输出异常返回NULL
 */
char* flJson_Dump(flJson* j);
```
**返回的字符串需要通过free函数进行释放**  


异常包括但不限于: 
- 字符串中有非可见字符且不在json标准内
- 递归深度过深
- 字符串拼接出错(内存不足)
