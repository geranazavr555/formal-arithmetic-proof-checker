#ifndef TASK5_DEBUG_OUTPUT_H
#define TASK5_DEBUG_OUTPUT_H

#include "expression.h"
#include <iostream>

using namespace std;

static string types[] = {
        "VAR",
        "IMPLICATION",
        "OR",
        "AND",
        "NOT",
        "ZERO",
        "MULTIPLY",
        "SUM",
        "FUNCTION",
        "VARIABLE",
        "PREDICATE",
        "EQUAL",
        "INCREMENT",
        "FOR_ALL",
        "EXISTS"};

static void _rec(ExpressionPtr expr, int depth = 0)
{
    string tab;
    for (int i = 0; i < depth; ++i)
        tab += "  ";
    cerr << tab << types[expr->type];
    if (expr->name != nullptr) {
        cerr << " " << expr->name;
    }
    cerr << endl;

    for (auto x : expr->exprs)
        _rec(x, depth + 1);
}

static void print(ExpressionPtr expr)
{
    _rec(expr);
}

#endif //TASK5_DEBUG_OUTPUT_H
