#ifndef TASK2_LOGIC_BASE_H
#define TASK2_LOGIC_BASE_H

#include <functional>
#include "expression.h"

namespace axiom_matching {
    void initialize();
    int match(const ExpressionPtr& ex);
}

#endif //TASK2_LOGIC_BASE_H
