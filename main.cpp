#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <mpfr.h>
#include "tokenize.h"


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
    CalculatorTokenize tokenize;
    BosonGrammarAnalyzer grammar;
    BosonSemanticsAnalyzer<mpfr_t> semantics;
    tokenize.tokenize(code);
    if (tokenize.get_error_line() != -1) {
        std::cerr << "[ERROR] Lexical error, line: " << tokenize.get_error_line() << std::endl;
        return -1;
    }
    BosonGrammar grammar_tree = grammar.grammar_analysis(tokenize.get_token_list());
    if (grammar_tree.get_error_index() != -1) {
        int line = tokenize.get_token_list()[grammar_tree.get_error_index()].line;
        std::cerr << "[ERROR] Grammar error:" << std::endl;
        for (auto &t: tokenize.get_token_list()) {
            if (line == t.line) {
                std::cerr << t.text << ' ';
            }
        }
        std::cerr << std::endl;
        return -1;
    }
    semantics.semantics_entity("set_variable", [&variables](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        char number[1024];
        mpfr_snprintf(number, 1024, "%.32Rf", node[1].get_data());
        variables[node[0].get_text()] = number;
        return BosonSemanticsNode<mpfr_t>::null_node();
    });
    semantics.semantics_entity("get_variable", [&variables](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        BosonSemanticsNode<mpfr_t> result;
        mpfr_init_set_str(result.get_data(), variables[node[0].get_text()].c_str(), 10, GMP_RNDD);
        return result;
    });
    semantics.semantics_entity("function", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
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
    semantics.semantics_entity("compute", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
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
    semantics.semantics_entity("number", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        BosonSemanticsNode<mpfr_t> result;
        mpfr_init_set_str(result.get_data(), node[0].get_text().c_str(), 10, GMP_RNDD);
        return result;
    });
    semantics.semantics_entity("expression", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> {
        return node[0];
    });
    semantics.semantics_analysis(grammar_tree.get_grammar_tree());
    return 0;
}