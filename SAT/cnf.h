//防止cnf.h文件被重复包含
#ifndef CNF_H
#define CNF_H

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
    Clause *clause;
}Formula;
Formula *loud_cnf(const char *filename);
void print_formular(const Formula *formula);
void free_formula(Formula *formula);

#endif
