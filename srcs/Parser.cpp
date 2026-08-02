#include "Parser.hpp"

Parser::Parser()
{
    // std::cout << "Parser Default Constructor called" << std::endl;
    return;
}

Parser::Parser(const Parser& other)
{
    // std::cout << "Parser Copy Constructor called" << std::endl;
    *this = other;
    return;
}

Parser &Parser::operator=(const Parser& other)
{
    if (this != &other)
        return (*this);
    // std::cout << "Parser Copy Assignment Operator called" << std::endl;
    return (*this);
}

Parser::~Parser()
{
    // std::cout << "Parser Destructor called" << std::endl;
    return;
}

ParsedCommand Parser::parse(const std::string& line)
{
    ParsedCommand result;

    size_t pos = line.find(' ');
    if (pos == std::string::npos)
    {
        result.command = line;
        return (result);
    }
    result.command = line.substr(0, pos);
    std::string parameters = line.substr(pos + 1);
    result.params.push_back(parameters);

    LOG_INFO << "command: " << result.command << " " << "params: " << result.params[0] ;
    return (result);
}