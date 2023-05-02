# 介绍

**YJson** 是一个轻量级的 JSON 序列化/反序列化 C++ 库。

# 使用方法

## 包含头文件

```cpp
#include <YJson/YJson.cpp>
```

## 示例

### 读取并反序列化

示例 JSON

```JSON
{
    "key_1": null,
    "key_2": 3.14,
    "key_3": "Hello World",
    "key_4": [
        "This", "is", 1, "List", "你好"
    ],
    "key_5": {
        "subkey_1": 2333,
        "subkey_2": true,
        "subkey_3": [
            114, 810
        ]
    }
}
```

示例代码

```cpp
#include <YJson/YJson.cpp>

using namespace std;

int main()
{
    system("chcp 65001");  // 将控制台编码设置为utf-8  Set the console encoding to utf-8

    // 读取并反序列化  Read & Deserialize
    auto obj = YJson::deserializeFromFile("Y:\\C++\\YJson\\example.json");

    cout << obj << endl;

    cout << obj["key_1"] << endl;
    cout << obj["key_2"] << endl;
    cout << obj["key_3"] << endl;
    cout << obj["key_4"] << endl;
    cout << obj["key_5"] << endl;

    cout << obj["key_4"][0] << endl;
    cout << obj["key_4"][1] << endl;
    cout << obj["key_4"][2] << endl;
    cout << obj["key_4"][3] << endl;
    cout << obj["key_4"][4] << endl;

    cout << obj["key_5"]["subkey_1"] << endl;
    cout << obj["key_5"]["subkey_2"] << endl;
    cout << obj["key_5"]["subkey_3"] << endl;

    bool b1 = obj["key_1"].isNull();
    cout << (b1 ? "true" : "false") << endl;
    bool b2 = obj["key_2"].isNull();
    cout << (b2 ? "true" : "false") << endl;

    auto lst = obj["key_4"].as<YJson::JsonListType>();  // typedef std::vector<Object> JsonListType;
    for (auto item : lst)
    {
        cout << item << " ";
    }
    cout << endl;

    auto dict = obj["key_5"].as<YJson::JsonDictType>();  // typedef std::map<std::string, Object> JsonDictType;
    for (auto item : dict)
    {
        cout << item.first << " ";
    }
    cout << endl;
}
```

控制台输出

```
{"key_1": null, "key_2": 3.14, "key_3": "Hello World", "key_4": ["This", "is", 1, "List", "你好"], "key_5": {"subkey_1": 2333, "subkey_2": true, "subkey_3": [114, 810]}}
null
3.14
"Hello World"
["This", "is", 1, "List", "你好"]
{"subkey_1": 2333, "subkey_2": true, "subkey_3": [114, 810]}
"This"
"is"
1
"List"
"你好"
invalid vector subscript
2333
true
[114, 810]
No such key: subkey_4
true
false
"This" "is" 1 "List" "你好" 
subkey_1 subkey_2 subkey_3 
```

### 序列化并写入

示例代码

```cpp
#include <YJson/YJson.cpp>


int main()
{
    YJson::Dict dict;
    YJson::Dict dict2;
    YJson::List list;

    dict["key_1"] = 3.14;
    dict["key_2"] = YJson::Object();  // null
    dict["key_3"] = "Hello World";


    list.push_back(YJson::Object(233.));
    list.push_back(YJson::Object((YJson::String)"你好"));
    dict["key_4"] = list;
  
    dict2["subkey"] = true;
    dict["key_5"] = dict2;

    YJson::serializeToFile("Y:\\C++\\YJson\\output.json", YJson::Object(dict), 4);
}
```

输出 JSON

```json
{
    "key_1": 3.14, 
    "key_2": null, 
    "key_3": true, 
    "key_4": [
        233, 
        "你好"
    ], 
    "key_5": {
        "subkey": true
    }
}
```

**在这里查看示例代码：**

```
./example.cpp
```

# 许可

```
MIT License

Copyright (c) 2023 YBR

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
