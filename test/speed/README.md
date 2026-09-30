# Json速度测试

>  [!Note]
>  这里使用的速度测试文件有来源于[jsonconsole.com](https://jsonconsole.com/sample-json/large-5mb), 大概5MB左右的json文件


## 介绍

这个测试旨在测试在大型json文本面前, 该解析器的解析能力如何  
  
但测试之前必须保证json文本是正确的且不可以嵌套过深, 否则会导致提前退出解析, 导致速度偏大  

## 增加测试

要增加测试直接在本文所在目录下的 `flJson_SpeedTest.c` 中增加函数即可  

### 规则

该测试以函数为单位, 函数的命令规则是 `SPEEDTEST_PARSE_FROM_{content}`, **content**是指json文件或者这个测试的来源(可以是任何东西); 函数必须是静态函数, 即在函数前面加上 `static` 关键字  
最后在 `main` 函数中加上新单元测试即可  
#### 提供的工具函数

| 函数原型 | 参数解释 | 功能 | 返回值解释 |
| --- | --- | --- | --- |
| `void FLJSON_SPEEDTEST_PARSE(const char* json_dir, size_t parse_cnt)` | `json_dir` : json的路径(必须基于 `test/speed/` ) | 用于测试解析(Parse)速度的函数 | 无 |
| | `parse_cnt` : 解析的次数, 如果文件比较小, 解析多次以保证数据可信 | | |


#### 例子

假设解析在 `test/speed/me/` 目录下的 `me.json` 文件  

```c

static void SPEEDTEST_PARSE_FROM_me() {
    /* 基于test/speed/路径来说, json的路径就是me/me.json */
    const char* json_dir = "me/me.json";
    FLJSON_SPEEDTEST_PARSE(json_dir, 1000);    
} 

int main() 
{
    SPEEDTEST_PARSE_FROM_me();

    /* main函数的其他内容... */
}

```


## 如何运行速度测试
将项目根目录最为工作目录, 然后输入下面的命令  
```shell
mkdir build
cd build
cmake -DFLJSON_BUILD_TEST=ON -DFLJSON_SPEED_TEST=ON ..        # BUILD_TEST选项由于开启测试, SPEED_TEST用于开启速度测试
make
ctest -V
```