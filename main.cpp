#include "parser.h"
#include "debug_output.h"
#include "axioms.h"

#include <iostream>
#include <vector>

using namespace std;

vector<ExpressionPtr> hypos;
ExpressionPtr target;

void parse_header()
{
    string header;
    getline(cin, header);

    size_t delim = header.find("|-");

    Parser plural_parser(header.substr(0, delim), true);
    ExpressionPtr cur = nullptr;
    while ((cur = plural_parser.parse()) != nullptr)
    {
        hypos.push_back(cur);
    }

    /*cerr << "Parsed hypos: " << hypos.size() << endl;
    for (auto x : hypos)
        print(x);*/

    target = parse_expression(header.substr(delim + 2));
    /*cerr << "Target:\n";
    print(target);*/
}

int match_hypothesis(ExpressionPtr const &ex)
{
    for (size_t i = 0; i < hypos.size(); ++i)
    {
        if (*ex == *hypos[i])
            return i;
    }
    return -1;
}

struct LineType
{
    static const char UNKNOWN = 0;
    static const char AXIOM = 1;
    static const char HYPOTHESIS = 2;
    static const char MODUS_PONENS = 3;
    static const char FOR_ALL_RULE = 4;
    static const char EXISTS_RULE = 5;
};

struct Info
{
    uint32_t index = 0;
    char type = 0;
    bool used = false;
};

vector<ExpressionPtr> lines;
unordered_map<Hash, Info> info_by_expr_hash;
unordered_map<Hash, vector<ExpressionPtr>> a_impl_b_by_b;

bool add_line(string const &s)
{
    auto line = parse_expression(s);
    //print(line);

    if (info_by_expr_hash.find(line->hash) != info_by_expr_hash.end())
    {
        delete line;
        return true;
    }

    if (line->type == Expression::Type::IMPLICATION)
    {
        a_impl_b_by_b[line->exprs[1]->hash].push_back(line);
    }

    // hypothesis?

    int hypo_number = match_hypothesis(line);
    if (hypo_number != -1)
    {
        Info info;
        info.type = LineType::HYPOTHESIS;
        info.index = lines.size() + 1;
        info_by_expr_hash[line->hash] = info;
        lines.push_back(line);
        return true;
    }

    // axiom?

    int axiom_number = axiom_matching::match(line);
    if (axiom_number != -1)
    {
        Info info;
        info.type = LineType::AXIOM;
        info.index = lines.size() + 1;
        info_by_expr_hash[line->hash] = info;
        lines.push_back(line);
        return true;
    }

    // MP?

    for (auto const &a_impl_b : a_impl_b_by_b[line->hash])
    {
        if (info_by_expr_hash.find(a_impl_b->exprs[0]->hash) != info_by_expr_hash.end())
        {
            Info info;
            info.type = LineType::MODUS_PONENS;
            info.index = lines.size() + 1;
            info_by_expr_hash[line->hash] = info;
            lines.push_back(line);
            a_impl_b_by_b.erase(line->hash);
            return true;
        }
    }

    // Forall-quainter rule

    if (line->type == Expression::Type::IMPLICATION && line->exprs[1]->type == Expression::Type::FOR_ALL)
    {
        char* varname = line->exprs[1]->name;
        auto A = line->exprs[0];
        auto B = line->exprs[1]->exprs[0];
        if (A->free_vars.find(varname) == A->free_vars.end())
        //if (!A->is_free_var(varname))
        {
            auto AimplB = new Expression(Expression::Type::IMPLICATION, A->deep_copy(), B->deep_copy());
            AimplB->count_hash();
            if (info_by_expr_hash.find(AimplB->hash) != info_by_expr_hash.end())
            {
                Info info;
                info.type = LineType::FOR_ALL_RULE;
                info.index = lines.size() + 1;
                lines.push_back(line);
                info_by_expr_hash[line->hash] = info;
                delete AimplB;
                return true;
            }
            delete AimplB;
        }
    }

    // Exists-quainter rule

    if (line->type == Expression::Type::IMPLICATION && line->exprs[0]->type == Expression::Type::EXISTS)
    {
        char* varname = line->exprs[0]->name;
        auto A = line->exprs[0]->exprs[0];
        auto B = line->exprs[1];
        if (B->free_vars.find(varname) == B->free_vars.end())
        //if (!B->is_free_var(varname))
        {
            auto AimplB = new Expression(Expression::Type::IMPLICATION, A->deep_copy(), B->deep_copy());
            AimplB->count_hash();
            if (info_by_expr_hash.find(AimplB->hash) != info_by_expr_hash.end())
            {
                Info info;
                info.type = LineType::EXISTS_RULE;
                info.index = lines.size() + 1;
                lines.push_back(line);
                info_by_expr_hash[line->hash] = info;
                delete AimplB;
                return true;
            }
            delete AimplB;
        }
    }

    delete line;
    return false;
}

void clean_up()
{
    delete target;
    for (auto x : hypos)
        delete x;
    for (auto x : lines)
        delete x;
}

int main()
{
    ios::sync_with_stdio(false);

    /*while (true)
    {
        string ss;
        getline(cin, ss);
        auto x = parse_expression(ss);
        print(x);
    }
    return 0;*/

    //freopen("tests/incorrect9.in", "r", stdin);

    axiom_matching::initialize();

    parse_header();

    string s;
    int i = 0;
    while (getline(cin, s))
    {
        i++;
        if (!add_line(s))
        {
            cout << "Line #" << i << " can’t be obtained\n";
            clean_up();
            return 0;
        }
        //cerr << i << endl;
    }

    if (*lines.back() != *target)
    {
        cout << "Required hasn't been proven\n";
    } else
    {
        cout << "Proof is correct\n";
    }

    clean_up();
    return 0;
}