#include <iostream>
#include <YJson/YJson.cpp>

using namespace std;

std::string rootPath = ".\\";

void DeserializeExample()
{
    // 读取并反序列化  Read & Deserialize
    auto obj = YJson::deserializeFromFile(rootPath + "example.json");

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

    auto list = obj["key_4"].as<YJson::List>();  // typedef std::vector<Object> List;
    for (auto item : list)
    {
        cout << item << " ";
    }
    cout << endl;

    auto dict = obj["key_5"].as<YJson::Dict>();  // typedef std::map<std::string, Object> Dict;
    for (auto item : dict)
    {
        cout << item.first << " ";
    }
    cout << endl;
}

void SerializeExample()
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

    YJson::serializeToFile(rootPath + "output.json", YJson::Object(dict), 4);
}

int main()
{
    system("chcp 65001");  // 将控制台编码设置为utf-8  Set the console encoding to utf-8

    DeserializeExample();
    SerializeExample();
}