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

/* 在内存中创建一个空公式（子句数组已分配但内容为空），
 * 供数独等模块直接构建 CNF 使用。失败返回 NULL。 */
Formula *formula_new(int variable_count, int clause_count);

/* 把 literals[0..literal_count-1] 写入第 clause_index 个子句。
 * 返回 1 成功，0 失败（参数非法或内存不足）。 */
int formula_set_clause(Formula *formula, int clause_index,
                       const int *literals, int literal_count);

void print_formula(const Formula *formula);

void free_formula(Formula *formula);

ClauseStatus is_clause_satisfied(const Clause *clause, const int *assignment);

FormulaStatus is_formula_satisfied(const Formula *formula, const int *assignment);

#endif
