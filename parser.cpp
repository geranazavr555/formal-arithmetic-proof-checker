#include "parser.h"
#include "debug_output.h"

using namespace std;

bool _is_var_char(char x)
{
    return ('a' <= x && x <= 'z') || ('0' <= x && x <= '9');
}

bool _is_predicate_char(char x)
{
    return ('A' <= x && x <= 'Z') || ('0' <= x && x <= '9');
}

bool _is_appendable_char(char x)
{
    return _is_var_char(x) || _is_predicate_char(x);
}

bool _is_skipable(char x)
{
    return !_is_appendable_char(x) && !(x == '-' || x == '>' || x == '|' || x == '&' || x == '(' || x == ')' || x == '!'
                                        || x == '-' || x == ',' || x == '.' || x == '+' || x == '=' || x == '*' ||
                                        x == '\'' || x == '@' || x == '?');
}

ExpressionPtr parse_expression(string s)
{
    return Parser(move(s)).parse();
}

char Parser::seek_next_char()
{
    while (index < expr.size() && _is_skipable(expr[index]))
        ++index;
    if (index == expr.size() - 1)
        return '\0';
    return expr[index];
}

void Parser::next_token()
{
    while (index < expr.size() && _is_skipable(expr[index]))
        ++index;

    if (index == expr.size())
    {
        token = Token::END;
        return;
    }

    switch (expr[index])
    {
        case '(':
            token = Token::OPEN_BRACE;
            //cerr << "open brace: " << index << endl;
            break;
        case ')':
            token = Token::CLOSE_BRACE;
            //cerr << "close brace: " << index << endl;
            break;
        case '-':
            token = Token::IMPLICATION;
            ++index;
            break;
        case '|':
            token = Token::OR;
            break;
        case '&':
            token = Token::AND;
            break;
        case '!':
            token = Token::NOT;
            break;
        case '@':
            token = Token::FOR_ALL;
            break;
        case '?':
            token = Token::EXISTS;
            break;
        case ',':
            token = Token::COMMA;
            break;
        case '+':
            token = Token::PLUS;
            break;
        case '*':
            token = Token::ASTERISK;
            break;
        case '0':
            token = Token::ZERO;
            break;
        case '.':
            token = Token::POINT;
            break;
        case '=':
            token = Token::EQUAL_SIGN;
            break;
        case '\'':
            token = Token::APOSTROPHE;
            break;
        default:
            size_t start = index;
            while (index < expr.size() && _is_appendable_char(expr[index]))
                ++index;
            --index;
            namestr = expr.substr(start, index - start + 1);

            if (_is_var_char(namestr[0]))
                token = Token::VAR_OR_FUNC;
            else
                token = Token::PREDICATE;
    }
    ++index;
}

ExpressionPtr Parser::multiply(
        ExpressionPtr fallback_result) // ⟨умножаемое⟩ ::= (‘a’ . . . ‘z’) {‘0’. . . ‘9’}* ‘(’ ⟨терм⟩ { ‘,’ ⟨терм⟩ }* ‘)’ |
//                                                   ⟨переменная⟩ | ‘(’ ⟨терм⟩ ‘)’ | ‘0’ | ⟨умножаемое⟩ ‘’’
{
    ExpressionPtr result = fallback_result;
    if (result == nullptr)
    {
        char next_char;
        switch (token)
        {
            case Token::VAR_OR_FUNC:
                next_char = seek_next_char();
                if (next_char == '(')
                {
                    //function
                    result = new Expression(Expression::Type::FUNCTION, namestr);
                    next_token();
                    while (token != Token::CLOSE_BRACE)
                    {
                        next_token();
                        result->add_expression(term());
                    }
                    next_token();
                } else
                {
                    result = new Expression(namestr);
                    next_token();
                }
                break;
            case Token::OPEN_BRACE:
                next_token();
                result = term();
                next_token();
                break;
            case Token::ZERO:
                result = new Expression(Expression::Type::ZERO);
                next_token();
                break;
        }
    }
    result->count_hash();
    while (token == Token::APOSTROPHE)
    {
        result = new Expression(Expression::Type::INCREMENT, result);
        result->count_hash();
        next_token();
    }
    return result;
}

ExpressionPtr Parser::add(ExpressionPtr fallback_result) // ⟨слагаемое⟩ ::= ⟨умножаемое⟩ | ⟨слагаемое⟩ ‘*’ ⟨умножаемое⟩
{
    auto result = (fallback_result == nullptr ? multiply() : fallback_result);
    while (token == Token::ASTERISK)
    {
        next_token();
        result = new Expression(Expression::Type::MULTIPLY, result, multiply());
        result->count_hash();
    }
    return result;
}

ExpressionPtr Parser::term(ExpressionPtr fallback_result) // ⟨терм⟩ ::= ⟨слагаемое⟩ | ⟨терм⟩ ‘+’ ⟨слагаемое⟩
{
    auto result = (fallback_result == nullptr ? add() : fallback_result);
    while (token == Token::PLUS)
    {
        next_token();
        result = new Expression(Expression::Type::SUM, result, add());
        result->count_hash();
    }
    return result;
}

ExpressionPtr Parser::predicate() // ⟨предикат⟩ ::= (‘A’ . . . ‘Z’) {‘0’. . . ‘9’}* [ ‘(’ ⟨терм⟩ { ‘,’ ⟨терм⟩ }* ‘)’ ] |
//                ⟨терм⟩ ‘=’ ⟨терм⟩
{
    ExpressionPtr result = nullptr;
    if (token == Token::PREDICATE)
    {
        result = new Expression(Expression::Type::PREDICATE, namestr);
        next_token();
        if (token == Token::OPEN_BRACE)
        {
            while (token != Token::CLOSE_BRACE)
            {
                next_token();
                result->add_expression(term());
            }
            next_token();
        }
    } else
    { // =
        /*if (token == Token::OPEN_BRACE)
            cerr << "-> Open brace!\n";*/
        auto left = term();
        if (token != Token::EQUAL_SIGN)
        {
            /*if (token == Token::CLOSE_BRACE)
                cerr << "<- Close brace!\n";*/
            //next_token();
            return left;
        }
        next_token();
        auto right = term();
        result = new Expression(Expression::Type::EQUAL, left, right);
    }
    result->count_hash();
    return result;
}

ExpressionPtr Parser::unary() // ⟨унарное⟩ ::= ⟨предикат⟩ | ‘!’ ⟨унарное⟩ | ‘(’ ⟨выражение⟩ ‘)’ |
//               (‘@’|‘?’) ⟨переменная⟩ ‘.’ ⟨выражение⟩
{
    string s;
    ExpressionPtr result;
    bool flag = false;
    switch (token)
    {
        case Token::NOT:
            next_token();
            result = new Expression(Expression::Type::NOT, unary());
            result->count_hash();
            break;
        case Token::FOR_ALL:
            next_token();
            result = new Expression(Expression::Type::FOR_ALL, namestr);
            next_token();
            next_token();
            result->add_expression(expression());
            result->count_hash();
            break;
        case Token::EXISTS:
            next_token();
            result = new Expression(Expression::Type::EXISTS, namestr);
            next_token();
            next_token();
            result->add_expression(expression());
            result->count_hash();
            break;
        case Token::OPEN_BRACE:
            next_token();
            result = expression();
            next_token();
            flag = true;
        default:
            if (!flag)
                result = predicate();

            /*if (token == Token::CLOSE_BRACE)
                cerr << "close brace!\n";*/

            while (token == Token::PLUS || token == Token::ASTERISK || token == Token::APOSTROPHE)
            {
                if (token == Token::PLUS)
                {
                    result = term(result);
                    /*if (token == Token::CLOSE_BRACE)
                        next_token();*/
                } else if (token == Token::ASTERISK)
                {
                    result = add(result);
                    /*if (token == Token::CLOSE_BRACE)
                        next_token();*/
                } else if (token == Token::APOSTROPHE)
                {
                    result = multiply(result);
                    /*if (token == Token::CLOSE_BRACE)
                        next_token();*/
                }
            }

            if (token == Token::EQUAL_SIGN)
            {
                next_token();
                auto right = term();
                result = new Expression(Expression::Type::EQUAL, result, right);
                result->count_hash();
            }
            break;
    }
    return result;
}

ExpressionPtr Parser::and_() // ⟨конъюнкция⟩ ::= ⟨унарное⟩ | ⟨конъюнкция⟩ ‘&’ ⟨унарное⟩
{
    auto result = unary();
    while (token == Token::AND)
    {
        next_token();
        result = new Expression(Expression::Type::AND, result, unary());
        result->count_hash();
    }
    return result;
}

ExpressionPtr Parser::or_() // ⟨дизъюнкция⟩ ::= ⟨конъюнкция⟩ | ⟨дизъюнкция⟩ ‘|’ ⟨конъюнкция⟩
{
    auto result = and_();
    while (token == Token::OR)
    {
        next_token();
        result = new Expression(Expression::Type::OR, result, and_());
        result->count_hash();
    }
    return result;
}

ExpressionPtr Parser::expression() // ⟨выражение⟩ ::= ⟨дизъюнкция⟩ | ⟨дизъюнкция⟩ ‘->’ ⟨выражение⟩
{
    auto result = or_();
    while (token == Token::IMPLICATION)
    {
        next_token();
        result = new Expression(Expression::Type::IMPLICATION, result, expression());
        result->count_hash();
    }
    return result;
}

Parser::Parser(std::string expression, bool plural) : expr(move(expression)), namestr(),
                                                      token(Token::NONE), index(0), _result(),
                                                      plural_expressions(plural)
{}

ExpressionPtr Parser::parse()
{
    if (!plural_expressions)
    {
        if (!_result)
        {
            next_token();
            _result = expression();
            if (token != Token::END)
                exit(43);
        }
        return _result;
    }
    else
    {
        if (token == Token::END)
            return nullptr;
        if (token == Token::NONE || token == Token::COMMA)
            next_token();
        if (token == Token::END)
            return nullptr;
        _result = expression();
        if (token != Token::COMMA && token != Token::END)
            exit(44);
        return _result;
    }
}
