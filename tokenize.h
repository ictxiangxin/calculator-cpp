#ifndef CALCULATOR_TOKENIZE_H
#define CALCULATOR_TOKENIZE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "parser.hpp"


class CalculatorTokenize {
public:
    CalculatorTokenize() = default;

    void tokenize(const char *code);

    void tokenize(std::string &code);

    std::vector<Token> get_token_list();

    int get_error_line();

    int get_error_state();

private:
    std::vector<Token> _token_list;
    int _error_line = -1;
    int _error_state = -1;

    std::unordered_map<char, std::string> _mark_type = {
            {'{', "t_brace_l"},
            {'}', "t_brace_r"},
            {'[', "t_bracket_l"},
            {']', "t_bracket_r"},
            {'(', "t_parenthesis_l"},
            {')', "t_parenthesis_r"},
            {'+', "t_plus"},
            {'-', "t_minus"},
            {'*', "t_times"},
            {'/', "t_divide"},
            {'=', "t_assign"},
            {',', "t_comma"},
            {'.', "t_dot"},
            {';', "t_semicolon"},
            {':', "t_colon"},
            {'^', "t_power"},
    };

    std::unordered_map<char, char> _escape = {
            {'a', '\a'},
            {'b', '\b'},
            {'f', '\f'},
            {'n', '\n'},
            {'r', '\r'},
            {'t', '\t'},
            {'v', '\v'},
    };
};


#endif //CALCULATOR_TOKENIZE_H
