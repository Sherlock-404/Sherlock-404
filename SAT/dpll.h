#ifndef DPLL_H
#define DPLL_H

#include "cnf.h"

/* 求解模式 */
#define DPLL_MODE_NAIVE     0  /* 基准（未优化）DPLL */
#define DPLL_MODE_OPTIMIZED 1  /* 优化 DPLL（默认） */

/* 求解 CNF 公式。
 * assignment 为长度 variable_count+1 的数组，下标 1..n；
 * 求解成功(SAT)时填入 TRUE_VALUE(1)/FALSE_VALUE(0)。
 * 返回：1=可满足(SAT)，0=不可满足(UNSAT)。
 */
int dpll(Formula *formula, int *assignment);

/* mode: DPLL_MODE_NAIVE 或 DPLL_MODE_OPTIMIZED */
int dpll_mode(Formula *formula, int *assignment, int mode);

/* 基准单元传播 / 单元文字查找（保留接口，供调用方兼容） */
int unit_propagation(Formula *formula, int *assignment);

int find_unit_literal(const Clause *clause, const int *assignment);

#endif
