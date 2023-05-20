#include <iostream>
#include <YJson/YJson.cpp>


using namespace std;

int main()
{
    system("chcp 65001");
    
    YJson::Object* obj = YJson::deserializeFromFile("Y://C++//YJson//example.json");
    cout << *obj << endl;
}