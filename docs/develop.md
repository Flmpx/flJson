# 开发文档

## Json本体的设计

从一开始的flJson本来是[这样设计](https://github.com/Flmpx/flJson/blob/08384e09de20426c869a12127141828a03c1d4e3/include/flJson.h)的  
- 整数(int), 浮点数(double), 布尔类型(bool)以及空值(null)直接存在Json结构体中  
- 字符串存指针, 且传入的字符串都会进行深拷贝
- 数组(Array) 以及 对象(Object) 单独抽象出来成为一个结构体, 分别叫做 `flArray` 和 `flObject`, 同时设置他们的层次和flJson是一样的, 都有相关的函数 

```c
// Json本体
typedef struct flJson flJson;

// Json数组
typedef struct flArray flArray;

// Json对象
typedef struct flObject flObject;
```

---

但是这种设计有一个问题, 就是每次创建一个Objecct或者Array的时候, 最终还是要把它插入到flJson中的, 还有此时二者的地位和flJson是一样的, 也就是都有相关的创建函数和解引(UnRef)函数, 那每次要创建一个要创建一个类型为Object的flJson的话, 步骤变得极其繁琐  

- 创建 `普通flJson` (int, double等)
- 调用New函数创建 `flObject`
- 向 `flObject` 中插入 `普通flJson`
- 由于 `flObect` 接管 `普通flJson`, 此时我们需要解引掉这个 `普通flJson`
- 调用 `Object类型的flJson` 的创建函数, 将 `flObject` 插入
- 此时 `Object类型的flJson` 接管了 `flObect`, 解引掉 `flObject`
- 终于得到了一个 `Object类型的flJson` 😥

如果还要在上面这种情况下再次插入其他 `普通flJson` 到 `Object类型的flJson` 中就更麻烦了, 这里就不说了  

---

之后, 询问了一下ai, 发现如果不抽象出对象和数组, 而是直接将他们嵌入到flJson中去, 他们不在属于单一体, 而是由flJson直接控制, [这种设计](https://github.com/Flmpx/flJson/blob/8848a25cff86f968189aebeea2059de892636fe2/include/flJson.h)似乎更好  
  
一来, 创建就没那么繁琐了, 不用去管单独的 `flObject`, 而是直接管 `Object类型的flJson`, 整个项目就只需要知道 **何类型的flJson** 就行了  
  
二来, flJson本来就有整数, 字符串(深拷贝)等基础类型的内存所有权, 如果按照上面这种设计, 那flJson对数组和对象是没有权限的, 必须由使用者手动转移, 这个过程是很繁琐的, 但是直接嵌入的话, flJson就管着所有类型的数据了, 这不就**统一六国**了吗?(NULL什么都不是🙂)

```c
// Json对象
typedef struct flObject flObject;
```

---

创建过程也变得更加简单  
  
- 创建一个 `普通flJson`
- 创建一个 `Object类型的flJson`
- 将 `普通flJson` 插入到 `Object类型的flJson`
- 如果不使用这个 `普通flJson`, 解引掉这个普通Json 
- 终于得到了一个 `Object类型的flJson` 😆


## int -> long long 的转变

最开始, 使用的是int, 毕竟int是最容易想到的  
  
但是在[解析字符串为flJson的代码实现](../src/flJsonParse.c)中的解析为整数的函数中, 需要调用将字符串转化为整数的函数, 但是标准库只提供了 `strtol`, `strtoul`, `strtoll` 以及 `strtoull` 这几个可用的函数, 并没有转化为int的, 强行转化为int反而不好  
  
与此同时, 在现在的flJson结构体中即使将 `4字节的int` 改为 `8字节的long long` 并不会对所占空间造成影响, 反而还可以让整数能表示的范围更大, 所以将int改为long long是利大于弊的  

## `\u0000` 的无奈

在C语言中, 如果要将 `\0` 安全的插入一个字符串中, 那就必须得让这个字符串带上长度信息, 虽然hmfocx有字符串类型, 但是这样引入一个新的抽象数据, 可能会让本就有点难理解的json树的构建过程变得更具难以理解  

并且 `\0` 基本不会使用到, 为了一个不常使用情况的而增加大量的使用负担, 这是不值当的  

综上, json中的字符串类型(Object的key以及字符串类型的json)不接受 `\u0000`