#ifndef TASK2_PARSER_H
#define TASK2_PARSER_H

#include "expression.h"
#include <string>
#include <memory>

class Parser {
    enum class Token {
        NONE, END, VAR_OR_FUNC, AND, OR, NOT, IMPLICATION, OPEN_BRACE, CLOSE_BRACE, FOR_ALL, EXISTS, POINT,
        PREDICATE, COMMA, PLUS, ASTERISK, ZERO, APOSTROPHE, EQUAL_SIGN
    };

    std::string expr;
    std::string namestr;
    Token token;
    size_t index;
    ExpressionPtr _result;
    bool plural_expressions;

private:
    char seek_next_char();
    void next_token();
    ExpressionPtr multiply(ExpressionPtr fallback_result = nullptr);
    ExpressionPtr add(ExpressionPtr fallback_result = nullptr);
    ExpressionPtr term(ExpressionPtr fallback_result = nullptr);
    ExpressionPtr predicate();
    ExpressionPtr unary();
    ExpressionPtr and_();
    ExpressionPtr or_();
    ExpressionPtr expression();

public:
    Parser(std::string expression, bool plural = false);
    ExpressionPtr parse();
};

ExpressionPtr parse_expression(std::string s);

#endif //TASK2_PARSER_H
