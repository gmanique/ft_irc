#ifndef PARSER_HPP
#define PARSER_HPP

#include "ft_irc.hpp"

struct ParsedCommand
{
    std::string command;
    std::vector<std::string> params;
};

class Parser {

    public:
        Parser();
        Parser(const Parser& other);
        Parser &operator=(const Parser& other);
        ~Parser();
        
        static ParsedCommand parse(const std::string& line);
};

#endif