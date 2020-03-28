/*
    Boson v1.5 - Grammar analyzer generator

        Author: ict
        Email:  ictxiangxin@hotmail.com
        Site:   https://github.com/ictxiangxin/boson

    File: "grammar_node.hpp"
    These codes ware generated by the boson C++ code generator.
*/

#ifndef _BOSON_GRAMMAR_NODE_HPP_
#define _BOSON_GRAMMAR_NODE_HPP_

namespace boson {
    class BosonGrammarNode {
    public:
        BosonGrammarNode() = default;

        explicit BosonGrammarNode(std::string &text) {
            this->set_text(text);
        }

        int get_reduce_number() {
            return this->_reduce_number;
        }

        void set_reduce_number(int reduce_number) {
            this->_reduce_number = reduce_number;
        }

        std::string &get_text() {
            return this->_text;
        }

        void set_text(std::string &text) {
            this->_text = text;
        }

        void append(const BosonGrammarNode &item) {
            this->_children.push_back(item);
        }

        void insert(int index, const BosonGrammarNode &item) {
            this->_children.insert(this->_children.begin() + index, item);
        }

        std::vector<BosonGrammarNode> &children() {
            return this->_children;
        }

        BosonGrammarNode &operator[](int index) {
            return this->_children[index];
        }

    private:
        int _reduce_number = -1;
        std::string _text{};
        std::vector<BosonGrammarNode> _children{};
    };
}

#endif