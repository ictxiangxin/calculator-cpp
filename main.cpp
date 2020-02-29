#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <mpfr.h>
#include "parser.hpp"


void usage() {
    std::cout << "Usage: calculator -[s|f] [code|file]" << std::endl;
    exit(0);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        usage();
    }
    std::string code;
    if (argv[1][0] == '-') {
        if (argv[1][1] == 's') {
            code = argv[2];
        } else if (argv[1][1] == 'f') {
            std::ifstream file;
            file.open(argv[2],  std::ifstream::in);
            if (file.good()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                code = buffer.str();
                file.close();
                buffer.clear();
            }
        } else {
            usage();
        }
    } else {
        usage();
    }
    mpfr_set_default_prec(128);
    std::unordered_map<std::string, std::string> variables;
    Tokenizer tokenizer;
    Parser parser;
    Interpreter<mpfr_t> interpreter;
    tokenizer.tokenize(code);
    if (tokenizer.tokenize(code) != tokenizer.no_error_line()) {
        std::cerr << "[ERROR] Lexical error, line: " << tokenizer.error_line() << std::endl;
        return -1;
    }
    BosonGrammar grammar_tree = parser.parse(tokenizer.token_list());
    if (grammar_tree.get_error_index() != grammar_tree.no_error_index()) {
        int line = tokenizer.token_list()[grammar_tree.get_error_index()].line;
        std::cerr << "[ERROR] Grammar error, line: " << line << std::endl;
        for (auto &t: tokenizer.token_list()) {
            if (line == t.line) {
                std::cerr << t.text << ' ';
            }
        }
        std::cerr << std::endl;
        return -1;
    }
    interpreter.register_action("set_variable", [&variables](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        char number[1024];
        mpfr_snprintf(number, 1024, "%.32Rf", node[1].get_data());
        variables[node[0].get_text()] = number;
        return BosonSemanticsNode<mpfr_t>::null_node();
    });
    interpreter.register_action("get_variable", [&variables](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        BosonSemanticsNode<mpfr_t> result;
        mpfr_init_set_str(result.get_data(), variables[node[0].get_text()].c_str(), 10, GMP_RNDD);
        return result;
    });
    interpreter.register_action("function", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        BosonSemanticsNode<mpfr_t> function_return;
        mpfr_init(function_return.get_data());
        std::string function_name = node[0].get_text();
        BosonSemanticsNode<mpfr_t> &arguments = node[1];
        if (function_name == "sqrt") {
            mpfr_sqrt(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
        } else if (function_name == "sin") {
            mpfr_sin(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
        } else if (function_name == "cos") {
            mpfr_cos(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
        } else if (function_name == "tan") {
            mpfr_tan(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
        } else if (function_name == "pow") {
            mpfr_pow(function_return.get_data(), arguments[0].get_data(), arguments[1].get_data(), GMP_RNDD);
        } else if (function_name == "print") {
            mpfr_printf("%.32Rf\n", arguments[0].get_data());
            return BosonSemanticsNode<mpfr_t>::null_node();
        }
        return function_return;
    });
    interpreter.register_action("compute", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        BosonSemanticsNode<mpfr_t> result;
        mpfr_init(result.get_data());
        BosonSemanticsNode<mpfr_t> &value_a = node[0];
        BosonSemanticsNode<mpfr_t> &op = node[1];
        BosonSemanticsNode<mpfr_t> &value_b = node[2];
        if (op.get_text() == "+") {
            mpfr_add(result.get_data(), value_a.get_data(), value_b.get_data(), GMP_RNDD);
        } else if (op.get_text() == "-") {
            mpfr_sub(result.get_data(), value_a.get_data(), value_b.get_data(), GMP_RNDD);
        } else if (op.get_text() == "*") {
            mpfr_mul(result.get_data(), value_a.get_data(), value_b.get_data(), GMP_RNDD);
        } else if (op.get_text() == "/") {
            mpfr_div(result.get_data(), value_a.get_data(), value_b.get_data(), GMP_RNDD);
        } else if (op.get_text() == "^") {
            mpfr_pow(result.get_data(), value_a.get_data(), value_b.get_data(), GMP_RNDD);
        }
        return result;
    });
    interpreter.register_action("number", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        BosonSemanticsNode<mpfr_t> result;
        mpfr_init_set_str(result.get_data(), node[0].get_text().c_str(), 10, GMP_RNDD);
        return result;
    });
    interpreter.register_action("expression", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        return node[0];
    });
    interpreter.execute(grammar_tree.get_grammar_tree());
    return 0;
}