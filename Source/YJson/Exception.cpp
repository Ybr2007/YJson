#pragma once
#include <string>
#include <exception>


namespace YJson
{
    class Exception : public std::exception
    {
    public:
        Exception() : __message("Error") {}
        
        Exception(std::string msg) : __message(msg) {}

        virtual ~Exception() throw() {}

        virtual const char *what() const throw()
        {
            return __message.c_str();
        }

    protected:
        std::string __message;
    };

    class ParsingException : public Exception
    {
    public:
        ParsingException(std::string msg, std::size_t pos)
        {
            this->__message = msg + " Position: " + std::to_string(pos);
            this->__pos = pos;
        }

        size_t pos() const
        {
            return this->__pos;
        }

    protected:
        std::size_t __pos;
    };
}