#pragma once
#include <YJson/Exception.cpp>
#include <YJson/Object.cpp>


namespace YJson
{
    typedef std::size_t Pos;

    class Parser
    {
    public:
        Object parse(std::string json);

    private:
        Pos __pointer;
        std::string __json;
        bool __finished;

        Object __parseObject();
        Object __parseNull();
        Object __parseNumber();
        Object __parseString();
        Object __parseBoolean();
        Object __parseList();
        Object __parseDict();

        inline char __curChar();
        bool __tryMovePtr();
        bool __isEmpty(char chr);
    };

    /*
    解析json文本

    Args:
        std::string json -> json文本

    Return:
        YJson::Object -> 解析得到的对象
    */
    Object Parser::parse(std::string json)
    {
        this->__json = json;
        this->__pointer = 0;
        this->__finished = false;

        if (this->__json.empty())
        {
            throw ParsingException("Empty string.", 0);
        }

        Object result = this->__parseObject();

        if (!this->__finished)
        {
            while (true)
            {
                if (!this->__isEmpty(this->__curChar()))
                {
                    throw ParsingException("A JSON should only have one root object.", this->__pointer);
                }
                if (!this->__tryMovePtr())
                {
                    break;
                }
            }
        }

        return result;
    }

    Object Parser::__parseObject()
    {
        // 跳过先导空白
        while (this->__isEmpty(this->__curChar()))
        {
            if(!__tryMovePtr())  // 从此处到结尾全是空白
            {
                throw ParsingException("Empty string.", this->__pointer);
            }
        }

        if (this->__curChar() == '{') return this->__parseDict();  // 匹配到 { 认定为dict
        if (this->__curChar() == '[') return this->__parseList();  // 匹配到 [ 认定为list
        if (this->__curChar() == '"') return this->__parseString();    // 匹配到 " 认定为string
        if (this->__curChar() == 't' || this->__curChar() == 'f') return this->__parseBoolean();  // 匹配到 t 或 f 认定为bool
        if (this->__curChar() == 'n') return this->__parseNull();  // 匹配到 n 认定为null
        if (std::isdigit(this->__curChar()) || this->__curChar() == '-') return this->__parseNumber();  // 匹配到数字认定为number
        throw ParsingException("Unknown object.", this->__pointer);
    }

    Object Parser::__parseNull()
    {
        std::string boolString = this->__json.substr(this->__pointer, 4);
        if (boolString == "null")
        {
            this->__pointer += 4;
            return Object();
        }
        else
        {
            throw ParsingException("Try to parse unknown object as null.", this->__pointer);
        }
    }

    Object Parser::__parseNumber()
    {
        Pos numberBeginPos = this->__pointer;
        Pos numberEndPos;  // 数字最后一位的位置
        
        bool foundDot = false;
        bool foundSignE = false;
        bool foundMinusSign = false;
        bool foundZeroAtBegin = false;
        Pos zeroAtBeginPos;

        while (true)
        {
            if (this->__curChar() == 'e')
            {
                if (foundMinusSign && this->__pointer == numberBeginPos + 1)
                {
                    throw ParsingException("Illegal number: Found 'e' following the minus sign.", this->__pointer);
                }
                if (!foundSignE)  // 检查是否已经存在'e'
                {
                    foundSignE = true;
                }
                else
                {
                    throw ParsingException("Illegal number: Multiple 'e' found.", this->__pointer);
                }
            }
            else if (this->__curChar() == '.')
            {
                if (foundMinusSign && this->__pointer == numberBeginPos + 1)
                {
                    throw ParsingException("Illegal number: Found dot following the minus sign.", this->__pointer);
                }
                if (!foundDot && !foundSignE)  // 只能有一个小数点，并且小数点不能在指数中
                {
                    foundDot = true;
                }
                else
                {
                    throw ParsingException("Illegal number: Found dot after the first dot or 'e'.", this->__pointer);
                }
            } 
            else if (this->__curChar() == '-')
            {
                // 如果负号不在数字的一个位则属于非法数字
                if (this->__pointer != numberBeginPos)
                {
                    throw ParsingException("Illegal number: The minus sign must at the begin of the number.", this->__pointer);
                }
                foundMinusSign = true;
            }
            else if (!std::isdigit(this->__curChar()))
            {
                numberEndPos = this->__pointer - 1;  // 如果这一位不是数字/负号/e/小数，那么数字的最后一位应该是当前位的前一位
                break;
            }
            else
            {
                if (this->__curChar() == '0')
                {
                    if ((foundMinusSign && this->__pointer == numberBeginPos + 1) || (!foundMinusSign && this->__pointer == numberBeginPos))
                    {
                        foundZeroAtBegin = true;
                        zeroAtBeginPos = this->__pointer;
                    }
                }
            }

            if (!this->__tryMovePtr())  // 如果这一位是合法的，但是是json的最后一个字符，那么数字的最后一位应该是当前位
            {
                numberEndPos = this->__pointer;
                break;
            }
        }

        if (foundZeroAtBegin && zeroAtBeginPos != numberEndPos)
        {
            throw ParsingException("Found zero at begin.", zeroAtBeginPos);
        }

       if (this->__json[numberEndPos] == 'e' || this->__json[numberEndPos] == '.' || 
            this->__json[numberEndPos] == '-')
        {
            throw ParsingException("Illegal number: 'e', '-' or '.' can not be the end of the number.", numberEndPos);
        }

        ObjectValue number = std::stod(this->__json.substr(numberBeginPos, numberEndPos - numberBeginPos + 1));
        return Object(number);
    }

    Object Parser::__parseString()
    {
        if (this->__curChar() != '"')
        {
            throw ParsingException("Left quotation mark not found.", this->__pointer);
        }

        if (!this->__tryMovePtr())  // 从左引号的下一个字符开始
        {
            throw ParsingException("Right quotation mark not found.", this->__pointer);
        }

        Pos leftQuotationMarkPos = this->__pointer;
        Pos rightQuotationMarkPos;
        bool escape = false;

        while (true)
        {   
            if (this->__curChar() == '\\')
            {
                escape = true;
            }

            else if (this->__curChar() == '"' && !escape)
            {
                rightQuotationMarkPos = this->__pointer;
                this->__tryMovePtr();  // 将右引号跳过
                break;
            }

            if (this->__curChar() != '\\' && escape)
            {
                escape = false;
            }

            if (!this->__tryMovePtr())
            {
                throw ParsingException("Right quotation mark not found.", this->__pointer);
            }
        }

        ObjectValue string = this->__json.substr(leftQuotationMarkPos, rightQuotationMarkPos - leftQuotationMarkPos);
        return Object(string);
    }

    Object Parser::__parseBoolean()
    {
        if (this->__curChar() == 't')
        {
            std::string boolString = this->__json.substr(this->__pointer, 4);
            if (boolString == "true")
            {
                this->__pointer += 4;
                return Object(ObjectValue(true));
            }
            else
            {
                throw ParsingException("Try to parse unknown object as boolean.", this->__pointer);
            }
        }

        if (this->__curChar() == 'f')
        {
            std::string boolString = this->__json.substr(this->__pointer, 5);
            if (boolString == "false")
            {
                this->__pointer += 5;
                return Object(ObjectValue(false));
            }
            else
            {
                throw ParsingException("Try to parse unknown object as boolean.", this->__pointer);
            }
        }
    }

    Object Parser::__parseList()
    {
        if (this->__curChar() != '[')
        {
            throw ParsingException("Left middle bracket not found.", this->__pointer);
        }

        if (!this->__tryMovePtr())  // 从左中括号的下一个字符开始
        {
            throw ParsingException("Right middle bracket not found.", this->__pointer);
        }

        List list;
        bool foundRightMiddleBracket = false;

        if (this->__curChar() == ']')
        {
            this->__tryMovePtr();
            return Object(ObjectValue(list));
        }

        while (true)
        {
            Object item = this->__parseObject();
            list.push_back(item);

            if (this->__finished)
            {
                throw ParsingException("Right middle bracket not found.", this->__pointer);
            }

            bool foundComma = false;
            while (true)
            {
                if (this->__curChar() == ',')
                {
                    if (!foundComma)
                    {
                        foundComma = true;
                    }
                    else
                    {
                        throw ParsingException("Item not found.", this->__pointer);
                    }
                }
                else if (!this->__isEmpty(this->__curChar()))
                {
                    if (this->__curChar() == ']')
                    {
                        foundRightMiddleBracket = true;
                        this->__tryMovePtr();
                    }
                    else if (!foundComma)
                    {
                        throw ParsingException("Comma not found.", this->__pointer);
                    }
                    break;
                }

                if (!this->__tryMovePtr())
                {
                    throw ParsingException("Right middle bracket not found.", this->__pointer);
                }
            }

            if (foundRightMiddleBracket)
            {
                if (!foundComma)
                {
                    break;
                }
                throw ParsingException("Comma at the end of the list.", this->__pointer);
            }
        }

        return Object(ObjectValue(list));
    }

    Object Parser::__parseDict()
    {
        if (this->__curChar() != '{')
        {
            throw ParsingException("Left brace not found.", this->__pointer);
        }

        if (!this->__tryMovePtr())  // 从左大括号的下一个字符开始
        {
            throw ParsingException("Right brace not found.", this->__pointer);
        }

        Dict dict;
        bool foundRightBrace = false;

        if (this->__curChar() == '}')  // 空dict
        {
            this->__tryMovePtr();
            return Object(ObjectValue(dict));
        }

        while (true)
        {
            Object key = this->__parseObject();
            if (key.type() != ObjectType::String)
            {
                throw ParsingException("Type of key is not string.", this->__pointer);
            }

            bool foundColon = false;
            while (true)
            {
                if (this->__curChar() == ':')
                {
                    this->__tryMovePtr();  // 跳过冒号
                    break;
                }

                if (!this->__tryMovePtr())
                {
                    throw ParsingException("Colon not found.", this->__pointer);
                }
            }

            if (this->__finished)  // 如果引号是最后一个字符
            {
                throw ParsingException("Value not found.", this->__pointer);
            }

            while (this->__isEmpty(this->__curChar()))
            {
                if (!this->__tryMovePtr())
                {
                    throw ParsingException("Right brace not found.", this->__pointer);
                }
            }

            if (this->__curChar() == '}') 
            {
                throw ParsingException("Value not found.", this->__pointer);
            }

            Object value = this->__parseObject();
            dict[std::get<std::string>(key.value())] = value;

            if (this->__finished)
            {
                throw ParsingException("Right brace not found.", this->__pointer);
            }

            bool foundComma = false;
            while (true)
            {
                if (this->__curChar() == ',')
                {
                    if (!foundComma)
                    {
                        foundComma = true;
                    }
                    else
                    {
                        throw ParsingException("Key not found.", this->__pointer);
                    }
                }
                else if (!this->__isEmpty(this->__curChar()))
                {
                    if (this->__curChar() == '}')
                    {
                        foundRightBrace = true;
                        this->__tryMovePtr();
                    }
                    else if (!foundComma)
                    {
                        throw ParsingException("Comma not found.", this->__pointer);
                    }
                    break;
                }

                if (!this->__tryMovePtr())
                {
                    throw ParsingException("Right brace not found.", this->__pointer);
                }
            }

            if (foundRightBrace)
            {
                if (!foundComma)
                {
                    break;
                }
                throw ParsingException("Comma at the end of the dict.", this->__pointer);
            }
        }

        return Object(ObjectValue(dict));
    }
    

    char Parser::__curChar()
    {
        return this->__json[this->__pointer];
    }

    bool Parser::__tryMovePtr()
    {
        if (this->__pointer < this->__json.size() - 1)
        {
            this->__pointer ++;
            return true;
        }
        this->__finished = true;
        return false;
    }

    bool Parser::__isEmpty(char chr)
    {
        return chr == ' ' || chr == '\n';
    }
}

