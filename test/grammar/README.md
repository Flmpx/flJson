# Json语法测试

>  [!Note]
>  这里使用的测试文件有来源于[JSONTestSuite](https://github.com/nst/JSONTestSuite)项目中 `test_parsing/` 目录下的Json文件, 使用[MIT许可证](https://github.com/nst/JSONTestSuite/blob/master/LICENSE)进行开发


## 介绍

这个测试旨在测试在 `flJson_Parse` 函数能不能正确的检测出json文本是否合法  


## 增加测试

要增加测试直接在本文所在目录下的 `flJson_GrammarTest.c` 中增加函数即可  

### 规则

该测试以函数为单位, 函数的命令规则是 `GRAMMARTEST_FROM_{content}`, **content**是指json文件或者这个测试的来源(可以是任何东西); 函数必须是静态函数, 即在函数前面加上 `static` 关键字  
最后在 `main` 函数中加上新单元测试即可  

#### 提供的工具函数

| 函数原型 | 参数解释 | 功能 | 返回值解释 |
| --- | --- | --- | --- |
| `void FLJSON_GRAMMARTEST(FL_TAG expect, const char* json_dir)` | `expect` : `FL_YES` `FL_NO` `FL_YES \| FL_NO` | 判断解析成功与否的正确性 | 无 |
| | `json_dir` : json的路径(必须基于 `test/grammar/` ) | | |

**注:** `FL_YES` 和 `FL_NO` 是提供的两个枚举, 分别代表预期是正确和错误的

#### 例子
假设解析在 `test/grammar/me/` 目录下的 `me.json` 文件, 假设预期是正确的  

```c

static void GRAMMARTEST_FROM_me() {
    /* 基于test/grammar/路径来说, json的路径就是me/me.json */
    const char* json_dir = "me/me.json";
    FLJSON_GRAMMARTEST(FL_YES, json_dir);    
} 

int main() 
{
    GRAMMARTEST_FROM_me();

    /* main函数的其他内容... */
}

```

## 如何运行语法测试
将项目根目录最为工作目录, 然后输入下面的命令  
```shell
mkdir build
cd build
cmake -DBUILD_TEST=ON -DGRAMMAR_TEST=ON ..        # BUILD_TEST选项由于开启测试, GRAMMAR_TEST用于开启语法测试
make
ctest -V
```


