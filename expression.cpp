#include "expression.h"
#include <utility>
#include <cstring>

using namespace std;

Expression::Expression(char type, ExpressionPtr left, ExpressionPtr right) :
        type(type), hash(), exprs({left, right})
{
    count_hash();
}

Hash _string_hash(char *s)
{
    const Hash P(193, 163);
    Hash result;
    int n = strlen(s);
    for (int i = 0; i < n; ++i)
    {
        auto tmp = static_cast<uint64_t>(s[i]);
        result = result * P + Hash(tmp, tmp);
    }
    return result;
}

Hash Expression::_function_arguments_hash()
{
    const Hash P(151, 131);
    Hash result;
    for (auto const &expr : exprs)
    {
        result = result * P + expr->hash;
    }
    return result;
}

Hash Expression::_predicate_arguments_hash()
{
    const Hash P(137, 173);
    Hash result;
    for (auto const &expr : exprs)
    {
        result = result * P + expr->hash;
    }
    return result;
}

void Expression::count_hash()
{
    // 2, 3, 17, 37, 61, 79, 89, 97,  107, 109, 113, 163, 167, 173, 181, 191, 197, 199
    // 2, 3, 17, 43, 73, 79, 97, 103, 113, 137, 151, 157, 167, 181, 191, 193, 197, 199
    set_free_vars();
    switch (type)
    {
        case Type::VAR:
            hash = _string_hash(name);
            break;
        case Type::IMPLICATION:
            hash = Hash(23, 13) * (exprs[0]->hash) + Hash(11, 7) * (exprs[1]->hash);
            break;
        case Type::OR:
            hash = Hash(7, 11) * (exprs[0]->hash) + Hash(19, 5) * (exprs[1]->hash);
            break;
        case Type::AND:
            hash = Hash(13, 47) * (exprs[0]->hash) + Hash(5, 23) * (exprs[1]->hash);
            break;
        case Type::NOT:
            hash = Hash(31, 19) * (exprs[0]->hash) + Hash(47, 31);
            break;
        case Type::ZERO:
            hash = Hash(67, 71);
            break;
        case Type::MULTIPLY:
            hash = Hash(127, 53) * (exprs[0]->hash) + Hash(157, 37) * (exprs[1]->hash);
            break;
        case Type::SUM:
            hash = Hash(131, 61) * (exprs[0]->hash) + Hash(71, 149) * (exprs[1]->hash);
            break;
        case Type::FUNCTION:
            hash = _string_hash(name) * Hash(53, 139) + _function_arguments_hash() * Hash(179, 41);
            break;
        case Type::PREDICATE:
            hash = _string_hash(name) * Hash(41, 179) + _predicate_arguments_hash() * Hash(139, 83);
            break;
        case Type::EQUAL:
            hash = Hash(43, 109) * (exprs[0]->hash) + Hash(83, 67) * (exprs[1]->hash);
            break;
        case Type::INCREMENT:
            hash = Hash(29, 59) * (exprs[0]->hash) + Hash(101, 107);
            break;
        case Type::FOR_ALL:
            hash = Hash(149, 101) * _string_hash(name) + Hash(59, 89) * exprs[0]->hash;
            break;
        case Type::EXISTS:
            hash = Hash(73, 29) * _string_hash(name) + Hash(103, 127) * exprs[0]->hash;
            break;
    }
}

Expression::~Expression()
{
    delete[] name;
    for (auto x: exprs)
        delete x;
}

Expression::Expression(char type, ExpressionPtr expr) :
        type(type), hash(), exprs({expr})
{
    count_hash();
}

char *_copy_char_ptr(const char *x)
{
    char *result = new char[strlen(x) + 1];
    strcpy(result, x);
    return result;
}

Expression::Expression(const string &name) : type(Type::VAR), name(_copy_char_ptr(name.c_str()))
{
    count_hash();
}

Expression::Expression(char type, const string &name) : type(type), name(_copy_char_ptr(name.c_str()))
{}

Expression::Expression(char type) : type(type)
{}

void Expression::add_expression(Expression *expr)
{
    exprs.push_back(expr);
}

bool Expression::is_free_var(const std::string &var)
{
    return free_vars.find(var) != free_vars.end() || vars.find(var) == vars.end();
}

void _copy_name(ExpressionPtr src, ExpressionPtr dest)
{
    dest->name = new char[strlen(src->name) + 1];
    strcpy(dest->name, src->name);
}

bool Expression::_free_to_substitute(char *old, Expression* target, std::map<std::string, int> &quantified_vars)
{
    if (is_free_var(old))
    {
        if (type == Type::VAR && strcmp(old, name) == 0)
        {
            for (auto const& free_var : target->free_vars)
            {
                if (quantified_vars[free_var] > 0)
                    return false;
            }
        }
        else
        {
            if (type == Type::FOR_ALL || type == Type::EXISTS)
                quantified_vars[name]++;

            for (auto x : exprs)
            {
                if (!x->_free_to_substitute(old, target, quantified_vars))
                    return false;
            }

            if (type == Type::FOR_ALL || type == Type::EXISTS)
                quantified_vars[name]--;
        }
    }
    return true;
}

bool Expression::free_to_substitute(char *old, Expression *target)
{
    std::map<std::string, int> quantified_vars;
    return _free_to_substitute(old, target, quantified_vars);
}

ExpressionPtr Expression::substitute(char *old, ExpressionPtr target)
{
    ExpressionPtr result = nullptr;
    if (is_free_var(old))
    {
        if (type == Type::VAR && name != nullptr && strcmp(old, name) == 0)
        {
            result = target->deep_copy();
        }
        else
        {
            result = new Expression(type);
            if (name != nullptr)
            {
                _copy_name(this, result);
            }
            for (auto const& x : free_vars)
                result->free_vars.insert(x);
            //copy(free_vars.begin(), free_vars.end(), result->free_vars.begin());
            for (auto x : exprs)
            {
                result->add_expression(x->substitute(old, target));
            }
            result->count_hash();
        }
    }
    else
        result = deep_copy();
    return result;
}

Expression *Expression::deep_copy()
{
    auto result = new Expression(type);
    if (name != nullptr)
    {
        _copy_name(this, result);
    }
    result->hash = hash;
    for (auto const& x : free_vars)
        result->free_vars.insert(x);
    for (auto const& x : vars)
        result->vars.insert(x);
//    copy(free_vars.begin(), free_vars.end(), result->free_vars.begin());

    for (auto x : exprs)
    {
        result->add_expression(x->deep_copy());
    }
    return result;
}

void Expression::set_free_vars()
{
    for (auto x : exprs)
    {
        for (const auto &var : x->free_vars)
            free_vars.insert(var);
        for (const auto &var : x->vars)
            vars.insert(var);
    }
    if (type == Type::EXISTS || type == Type::FOR_ALL)
        free_vars.erase(name);
    if (type == Type::VAR) {
        free_vars.insert(name);
        vars.insert(name);
    }
}

bool operator==(Expression const &a, Expression const &b)
{
    if (a.type != b.type)
        return false;
    return a.hash == b.hash;
}

bool operator!=(Expression const &a, Expression const &b)
{
    return !(a == b);
}

std::ostream &operator<<(std::ostream &s, Expression const &a)
{
    //s << a.to_string();
    return s;
}

namespace std
{
    size_t hash<Hash>::operator()(const Hash &k) const
    {
        return k.to_size_t();
    }
}