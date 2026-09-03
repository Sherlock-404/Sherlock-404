#ifndef STAR_SUDOKU_H
#define STAR_SUDOKU_H

#include "cnf.h"
#include "sudoku.h"
#include "sudoku_cnf.h"

/* 课程设计规定的 9 个星形格（row,col，1-based） */
extern const int star_cells[SUDOKU_SIZE][2];

/* 星形区域约束新增子句数：9 个数字 × 每数字(1+36) = 333 */
int star_sudoku_clause_count(void);

/* 检查星形格区域：已填数字必须互不重复（空格允许）。
 * 合法返回 1，否则返回 0。 */
int star_sudoku_valid(const Sudoku *sudoku);

/* 把星形约束写入 formula（普通数独 CNF 之后追加），从第 start 个子句开始。
 * 返回写入子句数，失败返回 -1。 */
int star_sudoku_fill_cnf(Formula *formula, int start);

/* 生成星形数独完整 CNF = 普通数独 CNF + 星形约束。
 * 返回公式指针，失败返回 NULL；使用完调用 free_formula 释放。 */
Formula *star_sudoku_make_cnf(const Sudoku *sudoku);

#endif
