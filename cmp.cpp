#include <string>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <YJson/YJson.cpp>

using namespace std;


template <class T>
void timeit(T&& f)
{
    time_t start = clock();
    f();
    cout << clock() - start << endl;
}


Json::Value& readFileJson()
{
    Json::Reader reader;
    Json::Value root;

    ifstream in("Y://C++//YJson//large_example2.json",ios::binary);

    reader.parse(in,root);

    in.close();

    return root;
}


int main(int argc,char* argv[])
{
    system("chcp 65001");
    try
    {
        timeit(
            []()
            {
                Json::Value& value = readFileJson();
                // cout << value.toStyledString() << endl;
            }
        );
        timeit(
            []()
            {
                YJson::Object& obj = YJson::deserializeFromFile("Y://C++//YJson//large_example2.json");
                // cout << obj << endl;
                delete &obj;
            }
        );
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

