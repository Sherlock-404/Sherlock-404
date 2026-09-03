#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sudoku_gen.h"
#include "sudoku_cnf.h"
#include "star_sudoku.h"
#include "dpll.h"

/* ---------- 简单随机数（xorshift32） ---------- */
static unsigned int g_rng = 123456789u;

static void rng_seed(unsigned int seed)
{
    g_rng = seed ? seed : 123456789u;
}

static unsigned int rng_next(void)
{
    unsigned int x = g_rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rng = x;
    return x;
}

/* 求解已构建好的公式并还原为棋盘；返回 1 成功，0 失败 */
static int solve_formula_to_grid(Formula *f, Sudoku *out)
{
    int n = f->variable_count;
    int *a = (int *)malloc(sizeof(int) * (n + 1));
    if (a == NULL) {
        return 0;
    }
    for (int i = 0; i <= n; i++) {
        a[i] = UNASSIGNED;
    }
    int ok = dpll(f, a) && sudoku_from_assignment(a, out);
    free(a);
    return ok;
}

/* 生成一个完整合法盘（普通或星形） */
static int make_full_grid(Sudoku *full, int with_star)
{
    Sudoku empty;
    memset(&empty, 0, sizeof(empty));
    Formula *f = with_star ? star_sudoku_make_cnf(&empty)
                           : sudoku_make_cnf(&empty);
    if (f == NULL) {
        return 0;
    }
    int ok = solve_formula_to_grid(f, full);
    free_formula(f);
    return ok;
}

/* 按行/列映射复制盘面 */
static void remap_grid(Sudoku *g, const int row_map[9], const int col_map[9])
{
    Sudoku t = *g;
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            g->grid[r][c] = t.grid[row_map[r]][col_map[c]];
        }
    }
}

/* 生成 0..n-1 的一个随机排列（n<=9） */
static void random_perm(int *arr, int n)
{
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }
    for (int i = n - 1; i > 0; i--) {
        int j = (int)(rng_next() % (unsigned int)(i + 1));
        int t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }
}

/* 分别生成行/列映射：
 * 三个 3 行带可整体换序，带内三行可换序；列同理。 */
static void make_maps(int row_map[9], int col_map[9])
{
    int band_order[3], stack_order[3];
    int in_rows[3][3], in_cols[3][3];
    random_perm(band_order, 3);
    random_perm(stack_order, 3);
    for (int k = 0; k < 3; k++) {
        random_perm(in_rows[k], 3);
        random_perm(in_cols[k], 3);
    }
    for (int band = 0; band < 3; band++) {
        for (int i = 0; i < 3; i++) {
            row_map[band * 3 + i] =
                band_order[band] * 3 + in_rows[band][i];
        }
    }
    for (int stack = 0; stack < 3; stack++) {
        for (int i = 0; i < 3; i++) {
            col_map[stack * 3 + i] =
                stack_order[stack] * 3 + in_cols[stack][i];
        }
    }
}

/* 数独等价变换（普通：可随机打乱宫与宫内行列；
 * 星形：只做数字重命名与旋转/翻转/转置，
 * 这些变换对星形格集合不变，保证星形约束仍成立） */
static void transform_full(Sudoku *g, int with_star)
{
    /* 数字随机重命名 */
    int perm[10];
    for (int d = 1; d <= 9; d++) {
        perm[d] = d;
    }
    for (int i = 9; i > 1; i--) {
        int j = (int)(rng_next() % (unsigned int)i) + 1;
        int t = perm[i];
        perm[i] = perm[j];
        perm[j] = t;
    }
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            g->grid[r][c] = perm[g->grid[r][c]];
        }
    }

    Sudoku t;
    if (rng_next() & 1u) {                      /* 转置 */
        t = *g;
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                g->grid[r][c] = t.grid[c][r];
            }
        }
    }
    if (rng_next() & 1u) {                      /* 上下翻转 */
        t = *g;
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                g->grid[r][c] = t.grid[8 - r][c];
            }
        }
    }
    if (rng_next() & 1u) {                      /* 左右翻转 */
        t = *g;
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                g->grid[r][c] = t.grid[r][8 - c];
            }
        }
    }

    if (!with_star) {
        int row_map[9], col_map[9];
        make_maps(row_map, col_map);
        remap_grid(g, row_map, col_map);
    }
}

int sudoku_has_unique_solution(const Sudoku *puzzle, int with_star)
{
    if (puzzle == NULL || !sudoku_valid(puzzle) ||
        (with_star && !star_sudoku_valid(puzzle))) {
        return 0;
    }

    /* 第一次：求一个解 */
    Formula *f1 = with_star ? star_sudoku_make_cnf(puzzle)
                            : sudoku_make_cnf(puzzle);
    if (f1 == NULL) {
        return -1;
    }
    Sudoku sol;
    if (!solve_formula_to_grid(f1, &sol)) {
        free_formula(f1);
        return 0;                               /* 无解 */
    }
    int base_count = f1->clause_count;
    free_formula(f1);

    /* 第二次：在原公式上附加“至少一格不同于 sol”的子句
     * 该子句为：¬v(r,c,sol[r][c]) 对 81 格取或。 */
    Formula *f2 = formula_new(SUDOKU_VAR_COUNT, base_count + 1);
    if (f2 == NULL) {
        return -1;
    }
    int written = with_star
                      ? (sudoku_fill_base(f2, 0, puzzle) +
                         star_sudoku_fill_cnf(f2, sudoku_base_clause_count(puzzle)))
                      : sudoku_fill_base(f2, 0, puzzle);
    if (written != base_count) {
        free_formula(f2);
        return -1;
    }

    int forbid[SUDOKU_SIZE * SUDOKU_SIZE];
    int k = 0;
    for (int r = 1; r <= 9; r++) {
        for (int c = 1; c <= 9; c++) {
            forbid[k++] = -sudoku_var(r, c, sol.grid[r - 1][c - 1]);
        }
    }
    if (!formula_set_clause(f2, base_count, forbid, 81)) {
        free_formula(f2);
        return -1;
    }

    int n = f2->variable_count;
    int *a = (int *)malloc(sizeof(int) * (n + 1));
    if (a == NULL) {
        free_formula(f2);
        return -1;
    }
    for (int i = 0; i <= n; i++) {
        a[i] = UNASSIGNED;
    }
    int second_sat = dpll(f2, a);       /* 再可满足 => 不止一个解 */
    free(a);
    free_formula(f2);
    return second_sat ? 0 : 1;
}

int sudoku_generate(Sudoku *puzzle, Sudoku *solution,
                    int with_star, int target_givens, unsigned int seed)
{
    if (puzzle == NULL || solution == NULL) {
        return 0;
    }

    if (seed == 0) {
        seed = (unsigned int)time(NULL);
    }
    rng_seed(seed);

    /* 1) 完整合法盘（普通 / 星形）并随机变换 */
    Sudoku full;
    if (!make_full_grid(&full, with_star)) {
        return 0;
    }
    transform_full(&full, with_star);
    *solution = full;
    *puzzle = full;

    int min_givens = target_givens > 0 ? target_givens : 17;
    if (min_givens > 81) {
        min_givens = 81;
    }

    /* 2) 挖洞：随机尝试移除提示，移掉后仍须唯一解 */
    int max_tries = 81 * 20;
    for (int tries = 0;
         tries < max_tries && sudoku_filled(puzzle) > min_givens;
         tries++) {
        int idx = (int)(rng_next() % 81u);
        int r = idx / 9;
        int c = idx % 9;
        if (puzzle->grid[r][c] == 0) {
            continue;
        }

        int backup = puzzle->grid[r][c];
        puzzle->grid[r][c] = 0;
        if (sudoku_has_unique_solution(puzzle, with_star) != 1) {
            puzzle->grid[r][c] = backup;        /* 挖掉会破坏唯一性，恢复 */
        }
    }

    return 1;
}
