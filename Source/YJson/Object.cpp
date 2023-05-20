#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <sstream>
#include <YJson/Exception.cpp>


namespace YJson
{    
    enum class ObjectType
    {
        Null, Number, String, Boolean, List, Dict
    };

    class Object;

    typedef double Number;
    typedef std::string String;
    typedef bool Bool;
    typedef std::vector<Object> List;
    typedef std::map<String, Object> Dict;

    typedef std::variant<
        std::monostate,  // Null
        Number,  // Number
        String,  // String
        Bool,  // Boolean
        List,  // List
        Dict  // Dict
    > ObjectValue;

    ObjectType variantIndexToType(unsigned short index)
    {
        switch (index)
        {
            case 0: return ObjectType::Null;
            case 1: return ObjectType::Number;
            case 2: return ObjectType::String;
            case 3: return ObjectType::Boolean;
            case 4: return ObjectType::List;
            case 5: return ObjectType::Dict;        
            default: throw Exception("Index has no corresponding object type");
        }
    }

    unsigned short typeToVariantIndex(ObjectType type)
    {
        switch (type)
        {
            case ObjectType::Null: return 0;
            case ObjectType::Number: return 1;
            case ObjectType::String: return 2;
            case ObjectType::Boolean: return 3;
            case ObjectType::List: return 4;
            case ObjectType::Dict: return 5;
            default: throw Exception("Type has no corresponding variant index");
        }
    }
    
    class Object
    {
    public:
        Object() : __value(ObjectValue()) {}  // Null
        Object(Number num) : __value(ObjectValue(num)) {} // Number
        Object(Bool num) : __value(ObjectValue(num)) {} // Boolean
        Object(const String& num) : __value(ObjectValue(num)) {} // String
        Object(const List& num) : __value(ObjectValue(num)) {} // List
        Object(const Dict& num) : __value(ObjectValue(num)) {} // Dict
        Object(ObjectValue value) : __value(value) {}  // Object Value

        Object& operator=(const ObjectValue& value)
        {
            this->__value = value;
            return *this;
        }

        ObjectType type() const
        {
            return variantIndexToType(this->__value.index());
        }

        bool isNull() const
        {
            return this->type() == ObjectType::Null;
        }

        ObjectValue& value()
        {
            return this->__value;
        }

        const ObjectValue& value() const
        {
            return this->__value;
        }

        void setValue(ObjectValue value)
        {
            this->__value = value;
        }

        Object& operator[](std::size_t index)
        {
            if (this->type() != ObjectType::List)
            {
                throw Exception("Only list object can be get item by index");
            }
            return (this->as<List>().at(index));
        }
        
        Object& operator[](std::string key)
        {
            if (this->type() != ObjectType::Dict)
            {
                throw Exception("Only list object can be get item by index");
            }
            Dict dictValue = this->as<Dict>();

            if (dictValue.count(key) > 0)
            {
                return dictValue[key];
            }

            throw Exception("No such key: " + key);
        }

        template<typename T>
        T as()
        {
            if (this->isNull())
            {
                throw Exception("Object is Null.");
            }

            try
            {
                T value = std::get<T>(this->__value);
                return value;
            }
            catch(const std::bad_variant_access& e)
            {
                throw Exception("Type does not match.");
            }
        }

        std::string toString(std::size_t indent=0, bool removeFollowingZerosForNumber=true,
                            std::size_t __indentLevel=1) const
        {
            if (this->type() == ObjectType::Null)
            {
                return "null";
            }
            if (this->type() == ObjectType::Number)
            {
                if (!removeFollowingZerosForNumber)
                {
                    return std::to_string(std::get<double>(this->__value));
                }
                
                std::ostringstream oss; 
                oss<<std::get<double>(this->__value); 
                return oss.str();
            }
            if (this->type() == ObjectType::String)
            {
                return '"' + std::get<std::string>(this->__value) + '"';
            }
            if (this->type() == ObjectType::Boolean)
            {
                bool value = std::get<bool>(this->__value);
                if (value)
                {
                    return "true";
                }
                return "false";
            }
            if (this->type() == ObjectType::List)
            {
                std::string str = "[";

                if (indent > 0)
                {
                    str += '\n';
                }

                List value =  std::get<List>(this->__value);
                for (std::size_t i = 0; i < value.size(); i ++)
                {
                    for (std::size_t _ = 0; _ < indent * __indentLevel; _ ++)
                    {
                        str += ' ';
                    }
                    
                    str += value[i].toString(indent, removeFollowingZerosForNumber, __indentLevel + 1);
                    if (i != value.size() - 1)
                    {
                        str += ", ";
                    }
                    if (indent > 0)
                    {
                        str += '\n';
                    }
                }

                if (indent > 0)
                {
                    for (std::size_t _ = 0; _ < indent * (__indentLevel - 1); _ ++)
                    {
                        str += ' ';
                    }
                }
                
                str += ']';

                return str;
            }
            if (this->type() == ObjectType::Dict)
            {
                std::string str = "{";

                if (indent > 0)
                {
                    str += '\n';
                }

                Dict dictValue =  std::get<Dict>(this->__value);

                std::size_t i = 0;
                for (auto item : dictValue)
                {
                    for (std::size_t _ = 0; _ < indent * __indentLevel; _ ++)
                    {
                        str += ' ';
                    }

                    str += "\"";
                    str += item.first;
                    str += "\"";
                    str += ": ";
                    str += item.second.toString(indent, removeFollowingZerosForNumber, __indentLevel + 1);

                    if (i != dictValue.size() - 1)
                    {
                        str += ", ";
                    }

                    if (indent > 0)
                    {
                        str += '\n';
                    }

                    i ++;
                }

                if (indent > 0)
                {
                    for (std::size_t _ = 0; _ < indent * (__indentLevel - 1); _ ++)
                    {
                        str += ' ';
                    }
                }

                str += '}';

                return str;
            }
        }
        
    private:
        ObjectValue __value;
    };

    std::ostream& operator<<(std::ostream& os, const Object& self)
    {
        os << self.toString();
        return os;
    }
}