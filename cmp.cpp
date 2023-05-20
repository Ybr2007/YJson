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

    ifstream in("Y://C++//YJson//large_example.json",ios::binary);

    reader.parse(in,root);

    in.close();

    return root;
}


int main(int argc,char* argv[])
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
            YJson::Object& obj = YJson::deserializeFromFile("Y://C++//YJson//large_example.json");
            // cout << obj << endl;
        }
    );
}