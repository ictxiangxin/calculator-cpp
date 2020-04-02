# 利用boson和mpfr构建高精度计算器的例子

* * *

## 编写词法和语法描述的`boson script`，对应文件`parser.boson`。

```
%parser_sparse_table yes; # 使用稀疏分析表。
%token_class_name Token; # 设置生成代码中的词元类名。
%lexer_class_name Tokenizer; # 设置生成代码中的词法分析器类名。
%parer_class_name Parser; # 设置生成代码中的语法分析器类名。
%interpreter_class_name Interpreter;  # 设置生成代码中的语义分析器类名。
%start_symbol calculator; # 指明起始分析符号。

# 词法描述部分。
t_symbol = <[_a-zA-Z][_a-zA-Z0-9]*>; # 普通符号只能有字母、数字和下划线构成，且不能以数字开头。
t_number = <[0-9]+(\.[0-9]*)?>; # 数字可以为整数和小数，不支持科学计数。
skip     = <[\ \t]+>@{skip}; # 过滤掉空白字符。
newline  = <\n\r|\n>@{skip, newline}; # 过滤掉换号并累计行号。


calculator: statement*; # 计算器脚本由若干语句构成。

statement: t_symbol '=' expression = set_variable($0, $2) # 语句可以是一个赋值语句，使用set_variable动作带入t_symbol和expression进行语义处理。
         | function # 语句可以是一个函数调用。
         ;

function: t_symbol '(' arguments ')' = function($0, $2); # 函数调用由函数名和参数列表组成，使用function动作带入t_symbol和arguments进行语义处理。

arguments: expression (',' expression)* = ($0, *$1($1)); # 参数列表由若干表达式构成，参数间用','隔开。

expression: expression ('+' | '-') expression_high = compute($0, *$1, $2) # 表达式的低优先级运算符：'+'、'-'，使用compute语义动作进行计算。
          | expression_high # 派生到高优先级表达式。
          ;

expression_high: expression_high ('*' | '/') expression_top = compute($0, *$1, $2) # 表达式的高优先级运算符：'*'、'/'。
               | expression_top # 派生到最高优先级表达式。
               ;

expression_top: expression_top '^' expression_final = compute($0, $1, $2) # 表达式的最高优先级运算符：'^'（指数运算）。
              | expression_final # 派生到最终表达式。
              ;

expression_final: '(' expression ')' = expression($1) # 最终表达式可以是由括号包围的子表达式。
                | function # 最终表达式可以是函数调用的返回值。
                | t_symbol = get_variable($0) # 最终表达式可以是一个变量值。
                | t_number = number($0)  # 最终表达式可以是一个立即数。
                ;
```

* * *

## 使用boson生成对应的分析器。

> 执行`boson parser.boson -a slr -l c++ -o parser`生成对应的分析器代码到`parser`目录。

```
Boson v1.5 - Grammar analyzer generator
    Author: ict
    Email:  ictxiangxin@hotmail.com
    URL:    https://github.com/ictxiangxin/boson

[Generate Analyzer Code]
    [1] Parse Boson Script... Done [0.0030s]
        > Commands Count: 6
        > Lexical Definition: Yes
        > Grammar Definition: Yes
    [2] Generate Lexical Analysis Table... Done [0.0582s]
        > Lexical Definition Count: 13
        > Character Set Size: 77
        > DFA State Count: 6
    [3] Generate Grammar Analysis Table... Done [0.0040s]
        > Algorithm: LALR
        > Grammar Sentence Count: 26
        > Non-Terminal Symbol Count: 16
        > Terminal Symbol Count: 12
        > PDA State Count: 37
        > Action Table Size/Sparse-Size (Rate): 481/168 (34.93%)
        > Goto Table Size/Sparse-Size (Rate): 592/48 (8.11%)
    [4] Generate Code... Done [0.0690s]
        > Language: C++
        > Mode: Integration
        > Checker: No
        > Generate Lexer: Yes
        > Generate Parser: Yes
        > Output Path: "parser"
[Complete!!! 0.1403s]
```

* * *

## 编写主程序`main.cpp`，调用分析器的词法分析类和语法分析类进行计算脚本分析。

> 调用词法分析和语法分析获得完整的抽象语法树。

```c++
Tokenizer tokenizer; // 创建词法分析器实例。
Parser parser; // 创建语法分析器实例。
Interpreter<mpfr_t> interpreter; // 创建语义分析器实例。
tokenizer.tokenize(code); // 对计算代码进行词法分析。
if (tokenizer.tokenize(code) != tokenizer.no_error_index()) { // 如果错误行存在则代码存在词法错误。
    std::cerr << "[ERROR] Lexical error, index: " << tokenizer.error_index() << std::endl; // 打印错误对应行号。
    return -1;
}
BosonGrammar grammar_tree = parser.parse(tokenizer.token_list()); // 进行语法分析。
if (grammar_tree.get_error_index() != -1) { // 如果错误行存在则代码存在语法错误。
    int line = tokenizer.token_list()[grammar_tree.get_error_index()].line; // 获取语法出错代码行号。
    std::cerr << "[ERROR] Grammar error, line: " << line << std::endl; // 打印出错代码行号。
    for (auto &t: tokenizer.token_list()) { // 打印出错代码行内容。
        if (line == t.line) {
            std::cerr << t.text << ' ';
        }
    }
    std::cerr << std::endl;
    return -1;
}
```

> 注册语义动作，以`function`函数调用语义动作为例。

```c++
interpreter.register_action("function", [](BosonSemanticsNode<mpfr_t> &node) -> BosonSemanticsNode<mpfr_t> { // 参数和返回值均为语义节点。
    BosonSemanticsNode<mpfr_t> function_return; // 创建函数返回值语义节点。
    mpfr_init(function_return.get_data()); // 初始化返回值mpfr值。
    std::string function_name = node[0].get_text(); // 根据语法定义，函数调用第一个参数为函数名。
    BosonSemanticsNode<mpfr_t> &arguments = node[1]; // 根据语法定义，函数调用第二个参数为参数列表。
    if (function_name == "sqrt") { // 如果函数名为sqrt，调用mpfr对应的开平方函数。
        mpfr_sqrt(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
    } else if (function_name == "sin") {
        mpfr_sin(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
    } else if (function_name == "cos") {
        mpfr_cos(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
    } else if (function_name == "tan") {
        mpfr_tan(function_return.get_data(), arguments[0].get_data(), GMP_RNDD);
    } else if (function_name == "pow") { // 该函数需2个参数。
        mpfr_pow(function_return.get_data(), arguments[0].get_data(), arguments[1].get_data(), GMP_RNDD);
    } else if (function_name == "print") { // 输出函数，该函数无返回值。
        mpfr_printf("%.32Rf\n", arguments[0].get_data());
        return BosonSemanticsNode<mpfr_t>::null_node(); // 无返回值，返回null语义节点。
    }
    return function_return; // 返回函数调用结果。
});
```

> 调用语义分析完成计算过程。

```c++
interpreter.execute(grammar_tree.get_grammar_tree());
```

* * *

## 测试程序。

> 使用cmake编译程序。

```cmake
cmake_minimum_required(VERSION 3.10)
project(calculator)

set(CMAKE_CXX_STANDARD 11)

add_executable(calculator main.cpp)

target_link_libraries(calculator mpfr)
```

> 编写测试计算脚本`test.txt`。

```
c = 299792458
m0 = 70
v = 0.1 * c

m = m0 / (1 - (v / c) ^ 2)

print(m)

print(sin(m * pow(2, m0)))

b = tan(m) / m
print(b)
```

> 执行`calculator -f a.txt`进行脚本计算，得到结果输出。

```
70.70707070707070707070707070707071
0.01501900015975742618890663327067
-0.66588482869753354169765171137603
```
