#ifndef DPLL_H
#define DPLL_H

#include "cnf.h"

int dpll(Formula *formula, int *assignment);

int unit_propagation(Formula *formula, int *assignment);

int find_unit_literal(const Clause *clause,const int *assignment);

#endif