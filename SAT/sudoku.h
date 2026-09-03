#ifndef SUDOKU_H
#define SUDOKU_H

/* 普通 9x9 数独 */
#define SUDOKU_SIZE 9
#define SUDOKU_BOX  3

/* 9x9x9 = 729 个布尔变元 */
#define SUDOKU_VAR_COUNT 729

/* 数独棋盘：grid[row][col]，0 表示空格，1~9 表示已填数字 */
typedef struct {
    int grid[SUDOKU_SIZE][SUDOKU_SIZE];
} Sudoku;

/* 语义变量 X(row,col,num) -> SAT 自然编号。
 * row/col/num 均为 1~9；编号 = (row-1)*81 + (col-1)*9 + num，范围 1..729。 */
int sudoku_var(int row, int col, int num);

/* 逆变换：var(1..729) -> row/col/num(1~9) */
void sudoku_decode_var(int var, int *row, int *col, int *num);

/* 从文件读取一个数独（读取前 81 个有效符号）：
 * 支持 sudoku17/csv 风格的 81 字符行、.ss 风格的排版（含 ! 与 --- 分隔符）、
 * Asterisk 官方文件（81 字符一行，'.' 为空格）。'0'/'.' 表示空格。
 * 返回 1 成功，0 失败或符号不足。 */
int sudoku_load(const char *filename, Sudoku *out);

/* 打印 9x9 棋盘（'.' 表示空格） */
void sudoku_print(const Sudoku *sudoku);

/* 已填格数 */
int sudoku_filled(const Sudoku *sudoku);

/* 合法性检查（不含星形约束）：
 * 每个格子 0~9；已填数字在同一行/列/宫不得重复。
 * 合法返回 1，否则返回 0。 */
int sudoku_valid(const Sudoku *sudoku);

/* 由 SAT 赋值还原棋盘。
 * assignment 长度须 >= 730；assignment[var]==TRUE_VALUE 表示该变元为真。
 * 还原完整且无冲突返回 1，否则返回 0。 */
int sudoku_from_assignment(const int *assignment, Sudoku *out);

#endif
