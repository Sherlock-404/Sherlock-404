#ifndef SUDOKU_CNF_H
#define SUDOKU_CNF_H

#include "cnf.h"
#include "sudoku.h"

/* 生成“恰好一个”约束：至少一个子句 + 两两互斥子句。
 * vars 为 n 个正整数（语义变量编号）。
 * 从 formula 的第 start 个子句开始写入，返回写入了多少个子句；
 * 失败返回 -1。 */
int sudoku_add_exactly_one(Formula *formula, int start,
                           const int *vars, int n);

/* 普通数独完整 CNF 的子句总数 = 基础约束 + 提示数个数 */
int sudoku_base_clause_count(const Sudoku *sudoku);

/* 把普通数独约束从第 start 个子句开始写入 formula。
 * 返回写入的子句数，失败返回 -1。 */
int sudoku_fill_base(Formula *formula, int start, const Sudoku *sudoku);

/* 生成普通数独的完整 CNF（729 变元；含提示数单子句）。
 * 返回公式指针，失败返回 NULL；使用完调用 free_formula 释放。 */
Formula *sudoku_make_cnf(const Sudoku *sudoku);

#endif
