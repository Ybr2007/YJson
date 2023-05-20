#pragma once
#include <fstream>
#include <sstream>
#include <YJson/Object.cpp>
#include <YJson/Parser.cpp>


namespace YJson
{
    const Object nullObject = Object();

    /*
    When you call this function and store into a value as obj, 
    you should call `delete &obj` somewhere.
    */
    Object& deserialize(std::string jsonString)
    {
        Parser parser;
        return *parser.parse(jsonString);
    }

    Object& deserializeFromFile(std::string filePath)
    {
        std::ifstream f(filePath, std::ios::in);  

        if(!f.is_open())
        {
            throw Exception("Failed to open file.");
        }

        std::stringstream buffer;  
        buffer << f.rdbuf();  
        std::string jsonString = buffer.str();

        f.close();

        return deserialize(jsonString);
    }

    Object& deserializeFromFile(std::ifstream f)
    {
        std::stringstream buffer;  
        buffer << f.rdbuf();  
        std::string jsonString = buffer.str();

        return deserialize(jsonString);
    }

    std::string serialize(Object obj, std::size_t indent=0,
                        bool removeFollowingZerosForNumber=true)
    {
        return obj.toString(indent, removeFollowingZerosForNumber);
    }

    void serializeToFile(std::string filePath, Object obj, std::size_t indent=0, 
                        bool removeFollowingZerosForNumber=true)
    {
        std::ofstream f(filePath, std::ios::out);
        f << serialize(obj, indent, removeFollowingZerosForNumber);
        f.close();
    }
    
    void serializeToFile(std::ofstream f, Object obj, std::size_t indent=0,
                        bool removeFollowingZerosForNumber=true)
    {
        f << serialize(obj, indent, removeFollowingZerosForNumber);
    }
}