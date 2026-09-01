//防止cnf.h文件被重复包含
#ifndef CNF_H
#define CNF_H

//定义枚举
typedef enum {
    FALSE_VALUE = 0,
    TRUE_VALUE = 1,
    UNASSIGNED = -1
} value;

//定义变量状态
typedef enum{
    CLAUSE_FALSE,
    CLAUSE_TRUE,
    CLAUSE_UNKNOWN
} ClauseStatus;

//定义公式状态
typedef enum{
    FORMULA_FALSE,
    FORMULA_TRUE,
    FORMULA_UNKNOWN
} FormulaStatus;

//每个Clause是一个子句
typedef struct Clause
{
   int *literals;
   int size;
}Clause;

//定义公式结构
typedef struct Formula
{
    int variable_count;//变量数
    int clause_count;//子句数
    Clause *clauses;
}Formula;

Formula *load_cnf(const char *filename);

void print_formula(const Formula *formula);

void free_formula(Formula *formula);

ClauseStatus is_clause_satisfied(const Clause *clause, const int *assignment);

FormulaStatus is_formula_satisfied(const Formula *formula, const int *assignment);

#endif
