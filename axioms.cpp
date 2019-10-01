#include "expression.h"
#include "parser.h"

#include <functional>
#include <cstring>

using namespace std;

namespace axiom_matching
{
    ExpressionPtr a13 = nullptr, a14 = nullptr, a15 = nullptr, a16 = nullptr, a17 = nullptr, a18 = nullptr;
    ExpressionPtr a19 = nullptr, a20 = nullptr;

    void initialize()
    {
        a13 = parse_expression("(a=b)->(a'=b')");
        a14 = parse_expression("(a=b)->(a=c)->(b=c)");
        a15 = parse_expression("(a'=b')->(a=b)");
        a16 = parse_expression("!(a'=0)");
        a17 = parse_expression("a+b' = (a+b)'");
        a18 = parse_expression("(a + 0) = a");
        a19 = parse_expression("a * 0 = 0");
        a20 = parse_expression("a * b' = a * b + a");
    }

    pair<bool, ExpressionPtr> find_substituted_term(char *var, ExpressionPtr before,
                                                    ExpressionPtr after,
                                                    ExpressionPtr result = nullptr) // P // P[x := term]
    {
        if (before->type == Expression::Type::VAR)
        {
            if (strcmp(var, before->name) == 0)
            {
                if (result == nullptr)
                    return {true, after};
                else
                    return {true, (*after == *result ? result : nullptr)};
            } else
                return {(after->type == before->type && strcmp(after->name, before->name) == 0), result};
        } else
        {
            if (before->is_free_var(var))
            {
                if (before->exprs.size() != after->exprs.size())
                    return {false, result};
                else
                {
                    ExpressionPtr _result = result;
                    for (size_t i = 0; i < before->exprs.size(); ++i)
                    {
                        auto tmp = find_substituted_term(var, before->exprs[i], after->exprs[i], _result);
                        if (!tmp.first)
                            return tmp;
                        _result = tmp.second;
                    }
                    return {true, _result};

                }
            } else
            {
                return {*after == *before, result};
            }
        }
    }

    const int AXIOMS_COUNT = 21;
    const function<bool(const ExpressionPtr &)> axioms[] = {
            [](const ExpressionPtr &ex) -> bool /// a -> (b -> a)
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto r = ex->exprs[1];
                if (r->type != Expression::Type::IMPLICATION)
                    return false;
                return *(ex->exprs[0]) == *(r->exprs[1]);
            },

            [](const ExpressionPtr &ex) -> bool /// (a -> b) -> ((a -> (b -> c)) -> (a -> c))
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[0];
                if (A->type != Expression::Type::IMPLICATION)
                    return false;
                auto B = ex->exprs[1];
                if (B->type != Expression::Type::IMPLICATION)
                    return false;
                auto C = B->exprs[1];
                if (C->type != Expression::Type::IMPLICATION)
                    return false;
                auto D = B->exprs[0];
                if (D->type != Expression::Type::IMPLICATION)
                    return false;
                auto E = D->exprs[1];
                if (E->type != Expression::Type::IMPLICATION)
                    return false;

                auto a1 = A->exprs[0];
                auto a2 = D->exprs[0];
                auto a3 = C->exprs[0];
                if (!(*a1 == *a2 && *a2 == *a3))
                    return false;

                auto b1 = A->exprs[1];
                auto b2 = E->exprs[0];
                if (*b1 != *b2)
                    return false;

                auto c1 = E->exprs[1];
                auto c2 = C->exprs[1];
                return *c1 == *c2;
            },

            [](const ExpressionPtr &ex) -> bool /// a -> (b -> (a & b))
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[1];
                if (A->type != Expression::Type::IMPLICATION)
                    return false;
                auto B = A->exprs[1];
                if (B->type != Expression::Type::AND)
                    return false;

                auto a1 = ex->exprs[0];
                auto a2 = B->exprs[0];
                if (*a1 != *a2)
                    return false;

                auto b1 = A->exprs[0];
                auto b2 = B->exprs[1];
                return *b1 == *b2;
            },

            [](const ExpressionPtr &ex) -> bool /// (a & b)-> a
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[0];
                if (A->type != Expression::Type::AND)
                    return false;
                auto a = A->exprs[0];
                auto c = ex->exprs[1];
                return *a == *c;
            },

            [](const ExpressionPtr &ex) -> bool /// (a & b) -> b
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[0];
                if (A->type != Expression::Type::AND)
                    return false;
                auto b = A->exprs[1];
                auto c = ex->exprs[1];
                return *b == *c;
            },

            [](const ExpressionPtr &ex) -> bool /// a -> (a | b)
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[1];
                if (A->type != Expression::Type::OR)
                    return false;
                auto a = A->exprs[0];
                auto c = ex->exprs[0];
                return *a == *c;
            },

            [](const ExpressionPtr &ex) -> bool /// b -> (a | b)
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[1];
                if (A->type != Expression::Type::OR)
                    return false;
                auto b = A->exprs[1];
                auto c = ex->exprs[0];
                return *b == *c;
            },

            [](const ExpressionPtr &ex) -> bool /// (a -> c) -> ((b -> c) -> ((a | b) -> c))
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[0];
                if (A->type != Expression::Type::IMPLICATION)
                    return false;
                auto B = ex->exprs[1];
                if (B->type != Expression::Type::IMPLICATION)
                    return false;
                auto C = B->exprs[0];
                if (C->type != Expression::Type::IMPLICATION)
                    return false;
                auto D = B->exprs[1];
                if (D->type != Expression::Type::IMPLICATION)
                    return false;
                auto E = D->exprs[0];
                if (E->type != Expression::Type::OR)
                    return false;

                auto a1 = A->exprs[0];
                auto a2 = E->exprs[0];
                if (*a1 != *a2)
                    return false;

                auto b1 = C->exprs[0];
                auto b2 = E->exprs[1];
                if (*b1 != *b2)
                    return false;

                auto c1 = A->exprs[1];
                auto c2 = C->exprs[1];
                auto c3 = D->exprs[1];
                return *c1 == *c2 && *c2 == *c3;
            },

            [](const ExpressionPtr &ex) -> bool /// (a -> b) -> ((a -> !b) -> !a)
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[0];
                if (A->type != Expression::Type::IMPLICATION)
                    return false;
                auto B = ex->exprs[1];
                if (B->type != Expression::Type::IMPLICATION)
                    return false;
                auto C = B->exprs[0];
                if (C->type != Expression::Type::IMPLICATION)
                    return false;
                auto D = B->exprs[1];
                if (D->type != Expression::Type::NOT)
                    return false;
                auto E = C->exprs[1];
                if (E->type != Expression::Type::NOT)
                    return false;

                auto a1 = A->exprs[0];
                auto a2 = C->exprs[0];
                auto a3 = D->exprs[0];
                if (!(*a1 == *a2 && *a2 == *a3))
                    return false;

                auto b1 = A->exprs[1];
                auto b2 = E->exprs[0];
                return *b1 == *b2;
            },

            [](const ExpressionPtr &ex) -> bool /// !!a -> a
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto A = ex->exprs[0];
                if (A->type != Expression::Type::NOT)
                    return false;
                auto B = A->exprs[0];
                if (B->type != Expression::Type::NOT)
                    return false;

                auto a1 = B->exprs[0];
                auto a2 = ex->exprs[1];
                return *a1 == *a2;
            },

            [](const ExpressionPtr &ex) -> bool /// (@x.P)->P[x := term]  term free to subst inst of x
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto Forall = ex->exprs[0];
                if (Forall->type != Expression::Type::FOR_ALL)
                    return false;
                auto Pterm = ex->exprs[1];
                auto P = Forall->exprs[0];
                char *varname = Forall->name;

                auto boolAndTerm = find_substituted_term(varname, P, Pterm);
                if (!boolAndTerm.first)
                    return false;
                if (boolAndTerm.second == nullptr)
                {
                    // only allowed: P == Pterm => term = x
                    return *P == *Pterm;
                } else
                {
                    auto PtermCanon = P->substitute(varname, boolAndTerm.second);
                    bool result = *Pterm == *PtermCanon && P->free_to_substitute(varname, boolAndTerm.second);
                    delete PtermCanon;
                    return result;
                }
            },

            [](const ExpressionPtr &ex) -> bool /// P[x := term]->?x.P   term free to subst inst of x
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto Exists = ex->exprs[1];
                if (Exists->type != Expression::Type::EXISTS)
                    return false;
                auto Pterm = ex->exprs[0];
                auto P = Exists->exprs[0];
                char *varname = Exists->name;

                auto boolAndTerm = find_substituted_term(varname, P, Pterm);
                if (!boolAndTerm.first)
                    return false;
                if (boolAndTerm.second == nullptr)
                    return *P == *Pterm;
                auto PtermCanon = P->substitute(varname, boolAndTerm.second);
                bool result = *Pterm == *PtermCanon;
                delete PtermCanon;
                return result;
            },

            [](const ExpressionPtr &ex) -> bool /// (a=b)->(a'=b')
            {
                return *ex == *a13;
            },

            [](const ExpressionPtr &ex) -> bool /// (a=b)->(a=c)->(b=c)
            {
                return *ex == *a14;
            },

            [](const ExpressionPtr &ex) -> bool /// (a'=b')->(a=b)
            {
                return *ex == *a15;
            },

            [](const ExpressionPtr &ex) -> bool /// !(a'=0)
            {
                return *ex == *a16;
            },

            [](const ExpressionPtr &ex) -> bool /// a+b' = (a+b)'
            {
                return *ex == *a17;
            },

            [](const ExpressionPtr &ex) -> bool /// (a + 0) = a
            {
                return *ex == *a18;
            },

            [](const ExpressionPtr &ex) -> bool /// a * 0 = 0
            {
                return *ex == *a19;
            },

            [](const ExpressionPtr &ex) -> bool /// a * b' = a * b + a
            {
                return *ex == *a20;
            },

            [](const ExpressionPtr &ex) -> bool /// ((T[x := 0]) & (@x.(T -> T[x := x']))) -> T   x free in T
            {
                if (ex->type != Expression::Type::IMPLICATION)
                    return false;
                auto And = ex->exprs[0];
                if (And->type != Expression::Type::AND)
                    return false;
                auto Tz = And->exprs[0];

                auto T = ex->exprs[1];
                auto Forall = And->exprs[1];
                if (Forall->type != Expression::Type::FOR_ALL || !T->is_free_var(Forall->name))
                    return false;

                auto InnerImpl = Forall->exprs[0];
                if (InnerImpl->type != Expression::Type::IMPLICATION)
                    return false;
                auto Ti = InnerImpl->exprs[0];
                if (*T != *Ti)
                    return false;
                auto Tinc = InnerImpl->exprs[1];

                char *varname = Forall->name;
                auto Zero = new Expression(Expression::Type::ZERO);
                Zero->count_hash();
                auto TzCanon = T->substitute(varname, Zero);
                if (*TzCanon != *Tz)
                {
                    delete TzCanon;
                    delete Zero;
                    return false;
                }

                auto Var = new Expression(Expression::Type::VAR, varname);
                Var->count_hash();
                auto VarInc = new Expression(Expression::Type::INCREMENT, Var);
                VarInc->count_hash();
                auto TincCanon = T->substitute(varname, VarInc);
                bool result = *Tinc == *TincCanon;
                delete TincCanon;
                delete VarInc;
                delete TzCanon;
                delete Zero;
                return result;
            }
    };

    int match(const ExpressionPtr &ex)
    {
        for (int i = 0; i < AXIOMS_COUNT; ++i)
        {
            bool result = axioms[i](ex);
            if (result)
                return i;
        }
        return -1;
    }
}