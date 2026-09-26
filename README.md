# flJson
这是一个基于我自己写的[hmfocx](https://github.com/Flmpx/hmfocx)库来写的一个可以解析Json文本的工具库  


## 快速开始
将 `include/` 和 `src/` 目录下的文件按照原组织方式复制到你的项目中  
  
复制如下代码  

```c
#if defined(__cplusplus) 
    extern "C" {
        #include "include/flJson.h"
    }
#else 
    #include "include/flJson.h"
#endif

int main()
{
    flJson* info = flJsonObject_New();

    flJson* author = flJsonString_New("Flmpx");
    flJsonObject_Add(info, "author", author);

    char* str = flJson_Dump(info);
    printf("%s\n", str);

    flJson_UnRef(info);
    flJson_UnRef(author);
    free(str);

    return 0;
}
```
  
运行结果大概是这样  
```txt
{"author":"Flmpx"}
```


## 介绍
### 文件目录  
采用的是经典的include和src目录  

```txt
.
├── CMakeLists.txt          # CMake构建静态库
├── README.md               # 简介
├── CHANGELOG.md            # 版本变更
├── docs            
│   ├── function.md         # 函数文档
│   ├── develop.md          # 开发文档
│   └── user.md             # 使用文档
├── include
│   └── flJson.h            # 头文件
├── src
│   ├── flJson.c            # 构建一个Json树的实现代码
│   ├── flJsonDump.c        # Json树转字符串的实现代码
│   └── flJsonParse.c       # 字符串转Json树的实现代码
└── test
    ├── README.md           # 测试相关简介
    ├── grammar             # 语法测试
    └── speed               # 速度测试
```
### Json

#### Json类型(总共七种)  
采用每种类型占据一个位(bit)的方式, 在判断flJson类型的时候可以同时判断多种  

```c
enum flJsonType {
    flJsonTypeLL            = 1L << 0,          // 整型
    flJsonTypeDouble        = 1L << 1,          // 浮点型
    flJsonTypeString        = 1L << 2,          // 字符串型
    flJsonTypeNull          = 1L << 3,          // 空
    flJsonTypeBool          = 1L << 4,          // 布尔型
    flJsonTypeObject        = 1L << 5,          // 对象
    flJsonTypeArray         = 1L << 6           // 数组
};
```


#### Json本体  
由于直接使用hmfocx, Json中的 `valArray_` 和 `valObject_` 分别来自于 hmfocx 的 `hm_arr` 和 `hm_map`  
为了不把过多的hmfocx容器的相关实现细节展示出来, 关于数组和对象的内嵌结构体只保留了必要的数据(大小数据和指针)  

```c
struct flJson {
    flJsonType type_;                   // Json的类型标志
    union {
        long long valLL_;               // 存整数
        double valDouble_;              // 存浮点数
        char* valString_;               // 存字符串指针
        bool valBool_;                  // 存布尔类型

        /* 存数组 */
        struct {
            flJson** array_;            // 存着Json指针的数组
            size_t size_;               // 元素数目
            size_t cap_;                // 容量
        } valArray_;

        /* 存对象 */
        struct {    
            void* entrys_;              // 存key和json的entry数组
            int* status_;               // 桶状态信息
            size_t size_;               // 元素数目
            size_t cap_;                // 容量
        } valObject_;
    };
    size_t refCount_;         // 引用计数, 当为0时即是释放内存时机
};
```

全局只有一个结构体, 就是 `flJson`, 我相信会很容易使用

## 构建

### 使用CMake集成到你的项目中

在 `CMakeLists.txt` 文件中添加下面代码  

```cmake
# ...
include(FetchContent)

FetchContent_Declare(
    flJson
    GIT_REPOSITORY https://github.com/Flmpx/flJson.git  # or git@github.com:Flmpx/flJson.git
    GIT_TAG v0.1.6
)

FetchContent_MakeAvailable(flJson)

# ...

target_link_library(your_executable PRIVATE flJson)
```

然后就是最常见的CMake构建流程了  

```shell
# 创建build文件夹
mkdir build

# 进入build文件夹
cd build

# CMake构建
cmake ..

# 执行Makefile文件
make
```

