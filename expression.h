#ifndef TASK2_EXPRESSION_H
#define TASK2_EXPRESSION_H

#include "hashing.h"
#include <memory>
#include <vector>
#include <set>
#include <map>

class Expression
{
public:
    struct Type
    {
        static const char VAR = 0;
        static const char IMPLICATION = 1;
        static const char OR = 2;
        static const char AND = 3;
        static const char NOT = 4;
        static const char ZERO = 5;
        static const char MULTIPLY = 6;
        static const char SUM = 7;
        static const char FUNCTION = 8;
        static const char PREDICATE = 10;
        static const char EQUAL = 11;
        static const char INCREMENT = 12;
        static const char FOR_ALL = 13;
        static const char EXISTS = 14;
    };

    char* name = nullptr;
    Hash hash;
    std::vector<Expression*> exprs;
    char type;
    std::set<std::string> free_vars;
    std::set<std::string> vars;

    Expression(char type, Expression* left, Expression* right);
    Expression(char type, Expression* expr);
    Expression(char type);
    Expression(const std::string& name);
    Expression(char type, const std::string& name);
    void add_expression(Expression* expr);
    void count_hash();
    void set_free_vars();
    bool is_free_var(const std::string& var);
    Hash _function_arguments_hash();
    Hash _predicate_arguments_hash();
    Expression* substitute(char *old, Expression* target);
    bool _free_to_substitute(char *old, Expression* target, std::map<std::string, int>& quantified_vars);
    bool free_to_substitute(char *old, Expression* target);
    Expression* deep_copy();
    ~Expression();
};

typedef Expression* ExpressionPtr;

bool operator==(Expression const &a, Expression const &b);
bool operator!=(Expression const &a, Expression const &b);

namespace std {
    template <>
    struct hash<Hash>
    {
        size_t operator()(const Hash& k) const;
    };
}

#endif //TASK2_EXPRESSION_H
