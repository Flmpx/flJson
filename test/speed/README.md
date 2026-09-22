# Json速度测试

>  [!Note]
>  这里使用的速度测试文件有来源于[jsonconsole.com](https://jsonconsole.com/sample-json/large-5mb), 大概5MB左右的json文件


## 介绍

这个测试旨在测试在大型json文本面前, 该解析器的解析能力如何  
  
但测试之前必须保证json文本是正确的且不可以嵌套过深, 否则会导致提前退出解析, 导致速度偏大  

## 增加测试

要增加测试直接在本文所在目录下的 `flJson_SpeedTest.c` 中增加函数即可  