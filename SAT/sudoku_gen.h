#ifndef SUDOKU_GEN_H
#define SUDOKU_GEN_H

#include "sudoku.h"

/* 判断题目是否“有解且唯一解”（用 DPLL：先求一解，再附加
 * “否定该解”的子句二次判定；第二次 UNSAT 即唯一）。
 * 返回 1=唯一解，0=无解或非唯一，-1=内部错误。 */
int sudoku_has_unique_solution(const Sudoku *puzzle, int with_star);

/* 自动生成数独题目：从完整合法盘随机变换后“挖洞”，
 * 每次挖洞都保证剩余题目仍唯一解。
 * with_star=1 生成星形数独（完整盘同时满足星形约束）。
 * target_givens 为期望保留的提示数下限（<=0 表示尽量挖）。
 * seed=0 时按当前时间自动选取随机种子。
 * 成功返回 1，puzzle/solution 填入结果；失败返回 0。 */
int sudoku_generate(Sudoku *puzzle, Sudoku *solution,
                    int with_star, int target_givens, unsigned int seed);

#endif
