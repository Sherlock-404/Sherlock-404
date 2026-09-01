#include <stdio.h>
#include "dpll.h"
#include <math.h>

//找到第一个未赋值的变量
int choose_variable(
    const Formula *formula,
    const int *assignment)
{
    for (int i = 1; i <= formula->variable_count; i++) {

        if (assignment[i] == UNASSIGNED) {
            return i;
        }
    }

    return 0;
}


//判断是否为单元子句
int find_unit_literal(const Clause *clause, const int *assignment)
{
    int unassigned_count = 0;
    int unit_literal = 0;

    for (int i = 0; i < clause->size; i++) {
        int literal = clause->literals[i];
        int variable = abs(literal);

        
        if ((literal > 0 && assignment[variable] == TRUE_VALUE) ||
                   (literal < 0 && assignment[variable] == FALSE_VALUE)) {
            return 0; // Clause is already satisfied
        }

        if (assignment[variable] == UNASSIGNED) {
            unassigned_count++;
            unit_literal = literal;
        } 


    }


    if (unassigned_count == 1) {
        return unit_literal;
    }

    return 0; // Not a unit clause
}



//单元传播
int unit_propagation(Formula *formula,int *assignment)
{
    int changed = 1;

    while (changed) {

        changed = 0;

        for (int i = 0;i < formula->clause_count;i++) {

            Clause *clause =&formula->clauses[i];

            int literal =find_unit_literal(clause,assignment);

            if (literal != 0) {

                int variable;

                if (literal > 0) {variable = literal;assignment[variable] = TRUE_VALUE;
                } else {
                    variable = -literal;
                    assignment[variable] = FALSE_VALUE;
                }

                changed = 1;
            }
        }

        /*
         * 检查有没有产生冲突
         */
        FormulaStatus status =is_formula_satisfied(formula,assignment);

        if (status == FORMULA_FALSE) {
            return 0;
        }
    }

    return 1;
}



//DPLL算法实现
int dpll(Formula *formula, int *assignment)
{
    // 进行单元传播
    if (!unit_propagation(formula, assignment)) {
        return 0;
    }

    //检查公式是否满足
    FormulaStatus status =is_formula_satisfied(formula, assignment);

    if (status == FORMULA_TRUE) {
        return 1;
    }

    if (status == FORMULA_FALSE) {
        return 0;
    }

    //选择一个未赋值的变量
    int variable =choose_variable(formula, assignment);

    /*
     * 尝试 TRUE
     */
    assignment[variable] = TRUE_VALUE;

    if (dpll(formula, assignment)) {
        return 1;
    }

    /*
     * TRUE 失败，尝试 FALSE
     */
    assignment[variable] = FALSE_VALUE;

    if (dpll(formula, assignment)) {
        return 1;
    }

    /*
     * 两种情况都失败
     */
    assignment[variable] = UNASSIGNED;

    return 0;
}