# 使用文档
这个文档将会介绍如何使用这个项目中的功能


## 创建Json树

注: 相关函数见[函数详述](./function.md)  


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

    flJson_UnRef(obj);
    flJson_UnRef(firstNameJson);
    flJson_UnRef(lastNameJson);
    flJson_UnRef(ageJson);
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

运行结果  
```txt
Name: flmpx hy
Age: 19
Name: hhmm hy
Age: 18
```