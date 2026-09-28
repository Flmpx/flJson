# 使用文档
这个文档将会介绍如何使用这个项目中的功能

注: 相关函数见[函数详述](./function.md)  

## 创建Json树


### 创建不同类型的Json
Json总共有七种类型, 基础类型有整型, 字符串型等; 而高级类型为对象和数组, 这两种类型可以插入其他类型的Json  

下面以 `创建一个字符串型和整型, 并将其插入到一个对象中` 为例子, 为了简化, 下面的代码不处理异常情况  

  
> 创建字符串型和整型

```cpp
flJson* num = flJsonLL_New(666);
flJson* str = flJsonString_New("#ae33f0")
```

> 创建对象

```cpp
flJson* obj = flJsonObject_New();
```

> 插入到对象中

```cpp
flJsonObject_Add(obj, "age", num);
flJsonObject_Add(obj, "color",  str);


/* 由于现在不需要使用到这个两个Json, 且对象也接管了, 所以解引掉 */
flJson_UnRef(num);
flJson_UnRef(str);
```

> 解引对象

```cpp
/* 使用完对象之后就可以解引了 */
flJson_UnRef(obj);
```


<br>
<br>
<br>
<br>
<br>
<br>



## 解析字符串

解析字符串有两个函数, 一个是直接解析, 另一个是根据传入的长度进行解析

### 函数声明
```c
flJson* flJson_Parse(const char* str);

flJson* flJson_ParseWithLength(const char* str, size_t len);
```

第一个是根据字符串的长度(c风格字符串)来进行解析  
第二个是可以指定要解析的字符串长度  

### 例子

**Json文本**  
```json
{
    "flmpx" : {
        "firstname" : "flmpx",
        "lastname" : "hy",
        "age" : 19
    },
    "hhmm" : {
        "firstname" : "hhmm",
        "lastname" : "hy",
        "age" : 18
    }
}
```

**程序**  
```c
#include <flJson.h>

void getPersonInfo(flJson* obj) {
    flJson* firstNameJson = flJsonObject_Get(obj, "firstname");
    char* firstName = flJsonString_Get(firstNameJson);
    
    flJson* lastNameJson = flJsonObject_Get(obj, "lastname");
    char* lastName = flJsonString_Get(lastNameJson);

    flJson* ageJson = flJsonObject_Get(obj, "age");
    long long* age = flJsonLL_Get(ageJson);

    printf("Name: %s %s\n", firstName, lastName);
    printf("Age: %lld\n", *age);
}

int main() 
{
    flJson* root = flJson_Parse("{ \"flmpx\" : {\"firstname\" : \"flmpx\", \"lastname\" : \"hy\", \"age\" : 19}, \"hhmm\" : {\"firstname\" : \"hhmm\", \"lastname\" : \"hy\", \"age\" : 18} }");
    getPersonInfo(flJsonObject_Get(root, "flmpx"));
    getPersonInfo(flJsonObject_Get(root, "hhmm"));

    flJson_UnRef(root);

    return 0;
}
```

**运行结果**  
```txt
Name: flmpx hy
Age: 19
Name: hhmm hy
Age: 18
```


<br>
<br>
<br>
<br>
<br>
<br>


## Json树转字符串

### 函数声明

```c
char* flJson_Dump(flJson* j);
```

### 例子

**程序**  
```c
#include <flJson.h>

int main() 
{
    flJson* info = flJsonObject_New();
    flJson* name = flJsonString_New("Flmpx");
    flJson* age = flJsonLL_New(19);

    flJsonObject_Add(info, "name", name);
    flJsonObject_Add(info, "age", age);

    char* str = flJson_Dump(info);
    printf("%s\n", str);

    flJson_UnRef(info);
    flJson_UnRef(name);
    flJson_UnRef(age);
    free(str);

    return 0;
}
```

**运行结果**  
```txt
{"age":19,"name":"Flmpx"}
```

<br>
<br>
<br>
<br>
<br>
<br>


## 迭代

库提供了对数组和对象类型的Json进行迭代的功能  
使用迭代器的一般流程是:  
- 初始化迭代器
- 判断当前迭代器索引是否有效
- 获取当前迭代器索引的值
- 将迭代器移至下一个位置

### 数组迭代

**程序**  
```c
#include <flJson.h>

void print(flJson* jll) {
    long long* ll = flJsonLL_Get(jll);
    printf("%lld ", *ll);
} 

int main() 
{
    flJson* arr = flJsonArray_New();
    for (int i = 0; i < 10; i++) {
        flJson* ele = flJsonLL_New(i * 10);
        flJsonArray_Add(arr, ele, flJsonArray_Size(arr));

        flJson_UnRef(ele);
    }

    /* 迭代 */
    flJsonArrayIter it;
    flJsonArrayIter_Init(&it, arr);
    while (flJsonArrayIter_HasCur(&it)) {
        print(flJsonArrayIter_Cur(&it));
        flJsonArrayIter_MoveNext(&it);
    }
    printf("\n");

    flJson_UnRef(arr);

    return 0;
}
```

**运行结果**

```txt
0 10 20 30 40 50 60 70 80 90
```


### 对象迭代

**程序**  
```c
#include <flJson.h>

void print(const char* key, flJson* js) {
    printf("key: %s, val: %s\n", key, flJsonString_Get(js));
} 

int main() 
{
    flJson* obj = flJsonObject_New();
    const char* keys[] = {"name", "age", "age"};
    const char* jsons[] = {"Flmpx", "19", "none"};
    int cnt = sizeof(keys) / sizeof(const char*);

    for (int i = 0; i < cnt; i++) {
        flJson* json = flJsonString_New(jsons[i]);
        flJsonObject_Add(obj, keys[i], json);

        flJson_UnRef(json);
    }

    flJsonObjectIter it;
    flJsonObjectIter_Init(&it, obj);
    while (flJsonObjectIter_HasCur(&it)) {
        print(flJsonObjectIter_CurKey(&it), flJsonObjectIter_CurVal(&it));
        flJsonObjectIter_MoveNext(&it);
    }

    flJson_UnRef(obj);

    return 0;
}
```

**运行结果**

```txt
key: age, val: none
key: name, val: Flmpx
```