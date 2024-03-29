/*
    Boson v1.5 - Grammar Analyzer Generator

        Author: ict
        Email:  ictxiangxin@hotmail.com
        Site:   https://github.com/ictxiangxin/boson

    File: "token.hpp"
    These codes ware generated by the boson C++ code generator.
*/

#ifndef _BOSON_TOKEN_HPP_
#define _BOSON_TOKEN_HPP_

namespace boson {
    using char_t = char;
    using string_t = std::string;

    class Token {
    public:
        Token() = default;

        Token(const string_t &text, const int line, std::string &symbol): text(text), line(line), symbol(symbol) {}

        Token(const char_t *text, const int line, const char *symbol): text(text), line(line), symbol(symbol) {}

        Token(const char_t *text, const int line, std::string &symbol): text(text), line(line), symbol(symbol) {}

        Token(const string_t &text, const int line, const char *symbol): text(text), line(line), symbol(symbol) {}

        string_t text{};
        int line = -1;
        std::string symbol{};
    };
}

#endif
