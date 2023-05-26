#pragma once
#include <YJson/Exception.cpp>
#include <YJson/Object.cpp>


namespace YJson
{
    typedef size_t Pos;


    class Parser
    {
    public:
        Object* parse(std::string_view jsonString);

    private:
        Pos __pointer;
        std::string __jsonString;
        bool __finished;

        Object* __parseObject();
        Object* __parseNull();
        Object* __parseNumber();
        Object* __parseString();
        Object* __parseTrue();
        Object* __parseFalse();
        Object* __parseList();
        Object* __parseDict();

        char __curChar();
        bool __tryMovePtr();

        bool __isBlank(char chr);
        bool __isSignE(char chr);
        bool __match(const char* pattern, size_t patternLength);
    };

    /*
    解析json文本

    Args:
        std::string jsonString -> json文本

    Return:
        YJson::Object -> 解析得到的对象
    */
    Object* Parser::parse(std::string_view jsonString)
    {
        this->__jsonString = jsonString;
        this->__pointer = 0;
        this->__finished = false;

        if (this->__jsonString.empty())
        {
            throw ParsingException("Empty string: The json string is empty.", 0);
        }

        Object* result = this->__parseObject();

        if (!this->__finished)
        {
            while (true)
            {
                if (!this->__isBlank(this->__curChar()))
                {
                    throw ParsingException("Multiple Root Object: A JSON should only have one root object.", this->__pointer);
                }
                if (!this->__tryMovePtr())
                {
                    break;
                }
            }
        }

        return result;
    }

    Object* Parser::__parseObject()
    {
        // 跳过先导空白
        while (this->__isBlank(this->__curChar()))
        {
            if(!__tryMovePtr())  // 从此处到结尾全是空白
            {
                throw ParsingException("Empty String: The remaining chars of the json string are all blank", this->__pointer);
            }
        }

        switch (this->__curChar())
        {
        case '{': return this->__parseDict();
        case '[': return this->__parseList();
        case '"': return this->__parseString();
        case 't': return this->__parseTrue();
        case 'f': return this->__parseFalse(); 
        case 'n': return this->__parseNull();

        default:
            if (std::isdigit(this->__curChar()) || this->__curChar() == '-') return this->__parseNumber();
            throw ParsingException("Unknown object.", this->__pointer);
        }
    }

    Object* Parser::__parseNull()
    {
        if (this->__match("null", 4))
        {
            return new Object();
        }
        else
        {
            throw ParsingException("Try to parse an unknown object as null.", this->__pointer);
        }
    }

    Object* Parser::__parseNumber()
    {
        Pos numberBeginPos = this->__pointer;  // 数字第一个字符的位置
        Pos numberEndPos;  // 数字最后一位的位置
        
        bool foundDot = false;
        bool foundSignE = false;
        bool foundMinusSign = false;
        bool foundZeroAtBegin = false;
        Pos beginningZeroPos;
        Pos dotPos;
        Pos signEPos;

        while (true)
        {
            if (this->__isSignE(this->__curChar()))
            {
                if ((foundMinusSign && this->__pointer == numberBeginPos + 1) || (!foundMinusSign && this->__pointer == numberBeginPos))
                {
                    throw ParsingException("Illegal number: Found 'e' following the minus sign.", this->__pointer);
                }
                if (!foundSignE)  // 检查是否已经存在'e'
                {
                    foundSignE = true;
                    signEPos = this->__pointer;
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
                    throw ParsingException("Illegal number: Found a dot following the minus sign.", this->__pointer);
                }
                if (!foundDot && !foundSignE)  // 只能有一个小数点，并且小数点不能在指数中
                {
                    foundDot = true;
                    dotPos = this->__pointer;
                }
                else
                {
                    throw ParsingException("Illegal number: Found a dot after the first dot or 'e'.", this->__pointer);
                }
            } 
            else if (this->__curChar() == '-')
            {
                // 如果负号不在数字的开头或指数的开头则属于非法数字
                if (this->__pointer != numberBeginPos && !(foundSignE && this->__pointer == signEPos + 1))
                {
                    throw ParsingException("Illegal number: The minus sign must be at the beginning of a number.", this->__pointer);
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
                    if ((foundMinusSign && this->__pointer == numberBeginPos + 1) || 
                        (!foundMinusSign && this->__pointer == numberBeginPos))
                    {
                        foundZeroAtBegin = true;
                        beginningZeroPos = this->__pointer;
                    }
                }
            }

            if (!this->__tryMovePtr())  // 如果这一位是合法的，但是是json的最后一个字符，那么数字的最后一位应该是当前位
            {
                numberEndPos = this->__pointer;
                break;
            }
        }

        if (foundZeroAtBegin && beginningZeroPos != numberEndPos && !(foundDot && beginningZeroPos == dotPos - 1))  // 先导0
        {
            throw ParsingException("Illegal number: Found a zero at beginning.", beginningZeroPos);
        }

       if (this->__isSignE(this->__jsonString[numberEndPos]) || this->__jsonString[numberEndPos] == '.' || 
            this->__jsonString[numberEndPos] == '-')
        {
            throw ParsingException("Illegal number: 'e', '-' or '.' can not be the end of the number.", numberEndPos);
        }

        ObjectValue number = std::stod(this->__jsonString.substr(numberBeginPos, numberEndPos - numberBeginPos + 1));
        return new Object(number);
    }

    Object* Parser::__parseString()
    {
        if (!this->__tryMovePtr())  // 从左引号的下一个字符开始
        {
            throw ParsingException("Right quotation mark not found.", this->__pointer);
        }

        Pos firstCharPos = this->__pointer;
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

        ObjectValue stringValue = this->__jsonString.substr(firstCharPos, rightQuotationMarkPos - firstCharPos);
        return new Object(stringValue);
    }

    Object* Parser::__parseTrue()
    {
        if (this->__match("true", 4))
        {
            return new Object(true);
        }
        else
        {
            throw ParsingException("Try to parse an unknown object as boolean.", this->__pointer);
        }
    }

    Object* Parser::__parseFalse()
    {
        if (this->__match("false", 5))
        {
            return new Object(false);
        }
        else
        {
            throw ParsingException("Try to parse an unknown object as boolean.", this->__pointer);
        }
    }

    Object* Parser::__parseList()
    {
        if (!this->__tryMovePtr())  // 从左中括号的下一个字符开始
        {
            throw ParsingException("Right middle bracket not found.", this->__pointer);
        }

        List list;
        bool foundRightMiddleBracket = false;

        if (this->__curChar() == ']')
        {
            this->__tryMovePtr();
            return new Object(list);
        }

        while (true)
        {
            Object* item = this->__parseObject();
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
                else if (!this->__isBlank(this->__curChar()))
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
                throw ParsingException("A comma at the end of the list.", this->__pointer);
            }
        }

        return new Object(list);
    }

    Object* Parser::__parseDict()
    {
        if (!this->__tryMovePtr())  // 从左大括号的下一个字符开始
        {
            throw ParsingException("Right brace not found.", this->__pointer);
        }

        Dict dict;
        bool foundRightBrace = false;

        if (this->__curChar() == '}')  // 空dict
        {
            this->__tryMovePtr();
            return new Object(dict);
        }

        while (true)
        {
            Object* key = this->__parseObject();
            if (key->type() != ObjectType::String)
            {
                throw ParsingException("Type of key is not the string.", this->__pointer);
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

            while (this->__isBlank(this->__curChar()))
            {
                if (!this->__tryMovePtr())
                {
                    throw ParsingException("Brace not found.", this->__pointer);
                }
            }

            if (this->__curChar() == '}') 
            {
                throw ParsingException("Value not found.", this->__pointer);
            }

            Object* value = this->__parseObject();
            dict[key->as<String>()] = value;

            delete key;

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
                else if (!this->__isBlank(this->__curChar()))
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

        return new Object(dict);
    }
    

    char Parser::__curChar()
    {
        return this->__jsonString[this->__pointer];
    }

    bool Parser::__tryMovePtr()
    {
        if (this->__pointer < this->__jsonString.size() - 1)
        {
            this->__pointer ++;
            return true;
        }
        this->__finished = true;
        return false;
    }

    bool Parser::__isBlank(char chr)
    {
        return chr == ' ' || chr == '\n';
    }

    bool Parser::__isSignE(char chr)
    {
        return chr == 'e' || chr == 'E';
    }

    bool Parser::__match(const char* pattern, size_t patternLength)
    {
        if (this->__pointer + patternLength >= this->__jsonString.size()) return false;
        for (Pos i = 0; i < patternLength; i ++)
        {
            if (pattern[i] != this->__curChar()) return false;
            if (!this->__tryMovePtr()) return false;
        }
        return true;
    }
}
