#ifndef SUDOKU_GAME_H
#define SUDOKU_GAME_H

#include "sudoku.h"

/* 判断在 (row,col) 填 num 是否与当前行/列/宫（及星形区域）冲突。
 * 只做局部冲突检查，不做全局可解性判断。row/col/num 均为 1-based。
 * num==0 只检查范围，不检查冲突。 */
int sudoku_game_move_ok(const Sudoku *board, int row, int col, int num,
                        int with_star);

/* 当前盘面是否仍然可解（调用 DPLL；盘面须无局部冲突）。
 * 返回 1=可解，0=不可解/无解，-1=盘面本身非法或内部错误。 */
int sudoku_game_solvable(const Sudoku *board, int with_star);

/* 求一个提示：调用 DPLL 求当前盘面的一个解，返回第一个空格及其应填数字。
 * 返回 1 成功（row、col、num 均为 1-based），0 表示已填满，-1 表示错误。 */
int sudoku_game_hint(const Sudoku *board, int with_star,
                     int *row, int *col, int *num);

/* 控制台交互游戏入口：内部自动生成一道普通/星形数独开始玩。
 * 返回 0 表示正常退出。 */
int sudoku_play(int with_star, unsigned int seed);

#endif
