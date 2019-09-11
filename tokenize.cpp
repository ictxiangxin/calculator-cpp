#include <string>
#include <stack>
#include <iostream>
#include "tokenize.h"


void CalculatorTokenize::tokenize(const char *code) {
    std::string temp_string(code);
    this->tokenize(temp_string);
}

void CalculatorTokenize::tokenize(std::string &code) {
    this->_token_list.clear();
    this->_error_line = -1;
    this->_error_state = -1;
    int state = 0;
    int line = 1;
    char string_edge = '\"';
    std::string text;
    for (size_t i = 0; i < code.length(); ++i) {
        char c = code[i];
        switch (state) {
            case 0:
                if (c == '\"' || c == '\'') {
                    string_edge = c;
                    state = 1;
                } else if ('0' <= c && c <= '9') {
                    text += c;
                    state = 2;
                } else if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
                    text += c;
                    state = 3;
                } else if (this->_mark_type.count(c)) {
                    text += c;
                    Token t{text, line, this->_mark_type[c]};
                    this->_token_list.push_back(t);
                    text = "";
                } else if (c == '#') {
                    state = 4;
                } else if (c == '\n') {
                    line++;
                    continue;
                } else if (c == ' ' || c == '\t' || c == '\r') {
                    continue;
                } else {
                    std::cout << (int)c;
                    state = -1;
                    break;
                }
                continue;
            case 1:
                if (c == string_edge) {
                    Token t(text, line, "t_string");
                    this->_token_list.push_back(t);
                    state = 0;
                    text = "";
                } else if (c == '\\') {
                    state = 5;
                } else if (c == '\n' || c == '\r') {
                    break;
                } else {
                    text += c;
                }
                continue;
            case 2:
                if (c == '.') {
                    text += c;
                    state = 6;
                } else if ('0' <= c && c <= '9') {
                    text += c;
                } else {
                    Token t(text, line, "t_number");
                    this->_token_list.push_back(t);
                    state = 0;
                    text = "";
                    i--;
                }
                continue;
            case 3:
                if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')) {
                    text += c;
                } else {
                    Token t(text, line, "t_symbol");
                    this->_token_list.push_back(t);
                    state = 0;
                    text = "";
                    i--;
                }
                continue;
            case 4:
                if (c == '\n') {
                    state = 0;
                }
                continue;
            case 5:
                if (this->_escape.count(c)) {
                    text += this->_escape[c];
                } else if (c == '\n' || c == '\r') {
                    break;
                } else {
                    text += c;
                }
                state = 1;
                continue;
            case 6:
                if ('0' <= c && c <= '9') {
                    text += c;
                } else {
                    Token t(text, line, "t_number");
                    this->_token_list.push_back(t);
                    state = 0;
                    text = "";
                    i--;
                }
                continue;
            default:
                break;
        }
    }
    Token end("", -1, "$");
    this->_token_list.push_back(end);
    if (state != 0) {
        this->_error_line = line;
        this->_error_state = state;
    }
}

std::vector<Token> CalculatorTokenize::get_token_list() {
    return this->_token_list;
}

int CalculatorTokenize::get_error_line() {
    return this->_error_line;
}

int CalculatorTokenize::get_error_state() {
    return this->_error_state;
}
