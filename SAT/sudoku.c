#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnf.h"
#include "sudoku.h"

/* 语义变量 X(row,col,num) -> SAT 自然编号 */
int sudoku_var(int row, int col, int num)
{
    return (row - 1) * 81 + (col - 1) * 9 + num;
}

/* SAT 自然编号 -> 行/列/数字 */
void sudoku_decode_var(int var, int *row, int *col, int *num)
{
    var -= 1;
    *row = var / 81 + 1;
    var %= 81;
    *col = var / 9 + 1;
    *num = var % 9 + 1;
}

static int to_digit(char ch)
{
    if (ch >= '1' && ch <= '9') {
        return ch - '0';
    }
    if (ch == '.' || ch == '0') {
        return 0;                       /* 空格 */
    }
    return -1;                          /* 非法符号 */
}

/* 读取前 81 个有效符号作为棋盘 */
int sudoku_load(const char *filename, Sudoku *out)
{
    if (out == NULL || filename == NULL) {
        return 0;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return 0;
    }

    int got = 0;
    int ch;
    int at_line_head = 1;               /* 用于跳过注释行 */
    while (got < 81 && (ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            at_line_head = 1;
            continue;
        }
        if (at_line_head) {
            if (ch == ' ' || ch == '\t' || ch == '\r') {
                continue;
            }
            if (ch == '/' || ch == '#') {
                while ((ch = fgetc(fp)) != EOF && ch != '\n') {
                    /* skip rest of comment line */
                }
                at_line_head = 1;
                continue;
            }
            at_line_head = 0;
        }

        int d = to_digit((char)ch);
        if (d < 0) {
            continue;                   /* 跳过分隔符、字母、空白等 */
        }
        int row = got / 9;
        int col = got % 9;
        out->grid[row][col] = d;
        got++;
    }
    fclose(fp);

    return got == 81;
}

void sudoku_print(const Sudoku *sudoku)
{
    if (sudoku == NULL) {
        return;
    }
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            if (sudoku->grid[r][c] == 0) {
                printf(".");
            } else {
                printf("%d", sudoku->grid[r][c]);
            }
            if (c == 2 || c == 5) {
                printf(" | ");
            } else if (c < 8) {
                printf(" ");
            }
        }
        printf("\n");
        if (r == 2 || r == 5) {
            printf("------+-------+------\n");
        }
    }
}

int sudoku_filled(const Sudoku *sudoku)
{
    int count = 0;
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            if (sudoku->grid[r][c] != 0) {
                count++;
            }
        }
    }
    return count;
}

/* 判断一列值（vals[0..8]）中，已填数字是否互不重复 */
static int unique_in_line(const int vals[SUDOKU_SIZE])
{
    int seen[SUDOKU_SIZE + 1] = {0};
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        int v = vals[i];
        if (v < 0 || v > 9) {
            return 0;
        }
        if (v != 0) {
            if (seen[v]) {
                return 0;
            }
            seen[v] = 1;
        }
    }
    return 1;
}

int sudoku_valid(const Sudoku *sudoku)
{
    if (sudoku == NULL) {
        return 0;
    }

    int row_vals[SUDOKU_SIZE], col_vals[SUDOKU_SIZE], box_vals[SUDOKU_SIZE];
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            row_vals[c] = sudoku->grid[r][c];
            col_vals[c] = sudoku->grid[c][r];
        }
        if (!unique_in_line(row_vals) || !unique_in_line(col_vals)) {
            return 0;
        }
    }

    for (int br = 0; br < SUDOKU_BOX; br++) {
        for (int bc = 0; bc < SUDOKU_BOX; bc++) {
            int k = 0;
            for (int i = 0; i < SUDOKU_BOX; i++) {
                for (int j = 0; j < SUDOKU_BOX; j++) {
                    box_vals[k++] =
                        sudoku->grid[br * SUDOKU_BOX + i][bc * SUDOKU_BOX + j];
                }
            }
            if (!unique_in_line(box_vals)) {
                return 0;
            }
        }
    }
    return 1;
}

int sudoku_from_assignment(const int *assignment, Sudoku *out)
{
    if (assignment == NULL || out == NULL) {
        return 0;
    }
    memset(out->grid, 0, sizeof(out->grid));

    for (int v = 1; v <= SUDOKU_VAR_COUNT; v++) {
        if (assignment[v] != TRUE_VALUE) {
            continue;
        }
        int row, col, num;
        sudoku_decode_var(v, &row, &col, &num);
        if (out->grid[row - 1][col - 1] != 0) {
            return 0;                   /* 同一格出现两个数字，异常 */
        }
        out->grid[row - 1][col - 1] = num;
    }

    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            if (out->grid[r][c] == 0) {
                return 0;               /* 有格未填，还原不完整 */
            }
        }
    }
    return 1;
}
