#include <stdio.h>
#include <stdlib.h>
#include "sudoku_cnf.h"

/* “恰好一个”：L1∨…∨Ln 以及 ¬Li∨¬Lj (i<j) */
int sudoku_add_exactly_one(Formula *formula, int start,
                           const int *vars, int n)
{
    if (formula == NULL || vars == NULL || n < 1) {
        return -1;
    }

    int cursor = start;

    /* 至少一个 */
    if (!formula_set_clause(formula, cursor++, vars, n)) {
        return -1;
    }

    /* 至多一个 */
    int pair[2];
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            pair[0] = -vars[i];
            pair[1] = -vars[j];
            if (!formula_set_clause(formula, cursor++, pair, 2)) {
                return -1;
            }
        }
    }
    return cursor - start;
}

/* 基础约束 4 类：81 格、9 行、9 列、9 宫，每类 81 个“单元”；
 * 每个单元 9 个候选，1+36=37 个子句。 */
#define SUDOKU_BASE_UNIT_COUNT (4 * 81)
#define SUDOKU_BASE_CLAUSES (SUDOKU_BASE_UNIT_COUNT * 37)

int sudoku_base_clause_count(const Sudoku *sudoku)
{
    if (sudoku == NULL) {
        return -1;
    }
    return SUDOKU_BASE_CLAUSES + sudoku_filled(sudoku);
}

int sudoku_fill_base(Formula *formula, int start, const Sudoku *sudoku)
{
    if (formula == NULL || sudoku == NULL || start < 0) {
        return -1;
    }

    int cursor = start;
    int vars[SUDOKU_SIZE];

    /* 1) 每格恰填一个数字 */
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            for (int n = 0; n < SUDOKU_SIZE; n++) {
                vars[n] = sudoku_var(r + 1, c + 1, n + 1);
            }
            int w = sudoku_add_exactly_one(formula, cursor, vars, SUDOKU_SIZE);
            if (w < 0) return -1;
            cursor += w;
        }
    }

    /* 2) 每行每个数字出现一次 */
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int n = 0; n < SUDOKU_SIZE; n++) {
            for (int c = 0; c < SUDOKU_SIZE; c++) {
                vars[c] = sudoku_var(r + 1, c + 1, n + 1);
            }
            int w = sudoku_add_exactly_one(formula, cursor, vars, SUDOKU_SIZE);
            if (w < 0) return -1;
            cursor += w;
        }
    }

    /* 3) 每列每个数字出现一次 */
    for (int c = 0; c < SUDOKU_SIZE; c++) {
        for (int n = 0; n < SUDOKU_SIZE; n++) {
            for (int r = 0; r < SUDOKU_SIZE; r++) {
                vars[r] = sudoku_var(r + 1, c + 1, n + 1);
            }
            int w = sudoku_add_exactly_one(formula, cursor, vars, SUDOKU_SIZE);
            if (w < 0) return -1;
            cursor += w;
        }
    }

    /* 4) 每个 3x3 宫每个数字出现一次 */
    for (int br = 0; br < SUDOKU_BOX; br++) {
        for (int bc = 0; bc < SUDOKU_BOX; bc++) {
            for (int n = 0; n < SUDOKU_SIZE; n++) {
                int k = 0;
                for (int i = 0; i < SUDOKU_BOX; i++) {
                    for (int j = 0; j < SUDOKU_BOX; j++) {
                        vars[k++] = sudoku_var(br * SUDOKU_BOX + i + 1,
                                               bc * SUDOKU_BOX + j + 1, n + 1);
                    }
                }
                int w = sudoku_add_exactly_one(formula, cursor, vars,
                                               SUDOKU_SIZE);
                if (w < 0) return -1;
                cursor += w;
            }
        }
    }

    /* 5) 提示数 -> 单子句 */
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            int d = sudoku->grid[r][c];
            if (d != 0) {
                int lit = sudoku_var(r + 1, c + 1, d);
                if (!formula_set_clause(formula, cursor++, &lit, 1)) {
                    return -1;
                }
            }
        }
    }

    return cursor - start;
}

Formula *sudoku_make_cnf(const Sudoku *sudoku)
{
    int total = sudoku_base_clause_count(sudoku);
    if (total < 0) {
        return NULL;
    }

    Formula *formula = formula_new(SUDOKU_VAR_COUNT, total);
    if (formula == NULL) {
        return NULL;
    }

    int written = sudoku_fill_base(formula, 0, sudoku);
    if (written != total) {
        free_formula(formula);
        return NULL;
    }
    return formula;
}
