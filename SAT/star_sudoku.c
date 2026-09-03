#include <stdio.h>
#include <stdlib.h>
#include "star_sudoku.h"

/* 星形格位置：25,33,37,52,55,58,73,77,85（row*10+col） */
const int star_cells[SUDOKU_SIZE][2] = {
    {2, 5},
    {3, 3},
    {3, 7},
    {5, 2},
    {5, 5},
    {5, 8},
    {7, 3},
    {7, 7},
    {8, 5}
};

int star_sudoku_clause_count(void)
{
    /* 9 个数字，每个数字一个“恰好一个”单元 = 9*(1+36) */
    return SUDOKU_SIZE * 37;
}

int star_sudoku_valid(const Sudoku *sudoku)
{
    if (sudoku == NULL) {
        return 0;
    }

    int seen[SUDOKU_SIZE + 1] = {0};
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        int r = star_cells[i][0] - 1;
        int c = star_cells[i][1] - 1;
        int d = sudoku->grid[r][c];
        if (d < 0 || d > SUDOKU_SIZE) {
            return 0;
        }
        if (d != 0) {
            if (seen[d]) {
                return 0;
            }
            seen[d] = 1;
        }
    }
    return 1;
}

int star_sudoku_fill_cnf(Formula *formula, int start)
{
    if (formula == NULL || start < 0) {
        return -1;
    }

    int cursor = start;
    int vars[SUDOKU_SIZE];

    for (int n = 0; n < SUDOKU_SIZE; n++) {
        for (int i = 0; i < SUDOKU_SIZE; i++) {
            vars[i] = sudoku_var(star_cells[i][0], star_cells[i][1], n + 1);
        }
        int w = sudoku_add_exactly_one(formula, cursor, vars, SUDOKU_SIZE);
        if (w < 0) {
            return -1;
        }
        cursor += w;
    }
    return cursor - start;
}

Formula *star_sudoku_make_cnf(const Sudoku *sudoku)
{
    if (sudoku == NULL) {
        return NULL;
    }

    int base_count = sudoku_base_clause_count(sudoku);
    if (base_count < 0) {
        return NULL;
    }

    int total = base_count + star_sudoku_clause_count();
    Formula *formula = formula_new(SUDOKU_VAR_COUNT, total);
    if (formula == NULL) {
        return NULL;
    }

    int written = sudoku_fill_base(formula, 0, sudoku);
    if (written != base_count) {
        free_formula(formula);
        return NULL;
    }
    written = star_sudoku_fill_cnf(formula, base_count);
    if (written != star_sudoku_clause_count()) {
        free_formula(formula);
        return NULL;
    }
    return formula;
}
