/* 数独模块自动化测试（不含 main.c 的交互代码）
 * 编译：gcc -O2 -std=c99 -o test_sudoku test_sudoku.c cnf.c dpll.c
 *       sudoku.c sudoku_cnf.c star_sudoku.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku.h"
#include "sudoku_cnf.h"
#include "star_sudoku.h"
#include "sudoku_gen.h"
#include "sudoku_game.h"
#include "dpll.h"

static int g_fails = 0;
static int g_checks = 0;

#define CHECK(cond, msg)                                          \
    do {                                                          \
        g_checks++;                                               \
        if (!(cond)) {                                            \
            g_fails++;                                            \
            printf("FAIL: %s (line %d)\n", msg, __LINE__);        \
        }                                                         \
    } while (0)

static int sudoku_from_text(const char *text, Sudoku *out)
{
    if (strlen(text) < 81) {
        return 0;
    }
    for (int i = 0; i < 81; i++) {
        char ch = text[i];
        int d;
        if (ch >= '1' && ch <= '9') {
            d = ch - '0';
        } else if (ch == '0' || ch == '.') {
            d = 0;
        } else {
            return 0;
        }
        out->grid[i / 9][i % 9] = d;
    }
    return 1;
}

/* 用 DPLL 解一个已构造好的棋盘（from 直接给出盘面） */
static int solve_grid(const Sudoku *puzzle, int with_star, Sudoku *out)
{
    Formula *f = with_star ? star_sudoku_make_cnf(puzzle)
                           : sudoku_make_cnf(puzzle);
    if (f == NULL) {
        return -1;
    }
    int n = f->variable_count;
    int *a = (int *)malloc(sizeof(int) * (n + 1));
    if (a == NULL) {
        free_formula(f);
        return -1;
    }
    for (int i = 0; i <= n; i++) {
        a[i] = UNASSIGNED;
    }
    int r = dpll(f, a);
    if (r) {
        r = sudoku_from_assignment(a, out);
    }
    free(a);
    free_formula(f);
    return r ? 1 : 0;
}

static int same_givens(const Sudoku *a, const Sudoku *b)
{
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (a->grid[r][c] != 0 && a->grid[r][c] != b->grid[r][c]) {
                return 0;
            }
        }
    }
    return 1;
}

static void test_encoding(void)
{
    for (int v = 1; v <= SUDOKU_VAR_COUNT; v++) {
        int row, col, num;
        sudoku_decode_var(v, &row, &col, &num);
        int back = sudoku_var(row, col, num);
        if (back != v) {
            g_fails++;
            g_checks++;
            printf("FAIL: encode roundtrip var=%d -> %d\n", v, back);
            return;
        }
        g_checks++;
    }
    CHECK(sudoku_var(1, 1, 1) == 1, "var(1,1,1)==1");
    CHECK(sudoku_var(2, 3, 5) == 104, "var(2,3,5)==104");
    CHECK(sudoku_var(9, 9, 9) == 729, "var(9,9,9)==729");
}

static void test_clause_counts(void)
{
    Sudoku p;
    memset(&p, 0, sizeof(p));
    p.grid[0][0] = 5;
    p.grid[3][4] = 7;

    Formula *f = sudoku_make_cnf(&p);
    CHECK(f != NULL, "build normal cnf");
    if (f) {
        CHECK(f->variable_count == 729, "normal vars == 729");
        CHECK(f->clause_count == 11988 + 2, "normal clauses = 11988+givens");
        free_formula(f);
    }

    f = star_sudoku_make_cnf(&p);
    CHECK(f != NULL, "build star cnf");
    if (f) {
        CHECK(f->variable_count == 729, "star vars == 729");
        CHECK(f->clause_count == 11988 + 2 + 333,
              "star clauses = normal + 333");
        free_formula(f);
    }
}

static void test_normal_solve(void)
{
    /* 课程 sudoku17.txt 的第一题（17 个提示数） */
    const char *p17 =
        "000000010400000000020000000000050407008000300001090000300400200050100000000806000";
    Sudoku puzzle, sol;
    CHECK(sudoku_from_text(p17, &puzzle), "parse 17-clue");
    CHECK(sudoku_valid(&puzzle), "17-clue basic valid");
    CHECK(sudoku_filled(&puzzle) == 17, "17 givens");

    int r = solve_grid(&puzzle, 0, &sol);
    CHECK(r == 1, "17-clue SAT");
    if (r == 1) {
        CHECK(sudoku_filled(&sol) == 81, "solution complete");
        CHECK(sudoku_valid(&sol), "solution normal-valid");
        CHECK(same_givens(&puzzle, &sol), "solution keeps givens");
    }

    /* 空盘一定可满足 */
    Sudoku empty, esol;
    memset(&empty, 0, sizeof(empty));
    int er = solve_grid(&empty, 0, &esol);
    CHECK(er == 1, "empty sudoku SAT");
    if (er == 1) {
        CHECK(sudoku_filled(&esol) == 81, "empty solution complete");
        CHECK(sudoku_valid(&esol) == 1, "empty solution valid");
    }

    /* 构造无解：第 1 行两个格子都提示 1 */
    Sudoku bad;
    memset(&bad, 0, sizeof(bad));
    bad.grid[0][0] = 1;
    bad.grid[0][1] = 1;
    CHECK(sudoku_valid(&bad) == 0, "conflict detected by validator");
    CHECK(solve_grid(&bad, 0, &sol) == 0, "conflict puzzle UNSAT");
}

static void test_star_solve(void)
{
    /* 正例：空盘 + 星形约束可解，且结果同时满足普通与星形约束 */
    Sudoku empty, sol;
    memset(&empty, 0, sizeof(empty));
    int r = solve_grid(&empty, 1, &sol);
    CHECK(r == 1, "star empty SAT");
    if (r == 1) {
        CHECK(sudoku_valid(&sol) == 1, "star solution normal-valid");
        CHECK(star_sudoku_valid(&sol) == 1, "star solution star-valid");
    }

    /* 关键验证：普通数独解不一定是星形解。
     * 做法：固定 (1,1)=d (d=1..9) 求普通解，找出一张星形格有重复数字的盘面 A；
     * 则“把 A 全部 81 格当提示”的普通 CNF 可满足，星形 CNF 不可满足。 */
    Sudoku a;
    int found = 0;
    for (int d = 1; d <= 9 && !found; d++) {
        Sudoku p;
        memset(&p, 0, sizeof(p));
        p.grid[0][0] = d;
        if (solve_grid(&p, 0, &a) == 1 && !star_sudoku_valid(&a)) {
            found = 1;
        }
    }
    CHECK(found == 1, "found normal grid whose star cells repeat");
    if (found) {
        CHECK(sudoku_valid(&a) == 1, "grid A is a valid normal sudoku");
        CHECK(star_sudoku_valid(&a) == 0, "grid A violates star region");

        Sudoku same;
        CHECK(solve_grid(&a, 0, &same) == 1, "A as full hints: normal SAT");
        CHECK(solve_grid(&a, 1, &same) == 0, "A as full hints: star UNSAT");
    }
}

static void test_loader(void)
{
    const char *p81 =
        "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    const char *ss_style =
        "5 3 . | . 7 . | . . .\n"
        "6 . . | 1 9 5 | . . .\n"
        ". 9 8 | . . . | . 6 .\n"
        "------+-------+------\n"
        "8 . . | . 6 . | . . 3\n"
        "4 . . | 8 . 3 | . . 1\n"
        "7 . . | . 2 . | . . 6\n"
        "------+-------+------\n"
        ". 6 . | . . . | 2 8 .\n"
        ". . . | 4 1 9 | . . 5\n"
        ". . . | . 8 . | . 7 9\n";

    const char *comment_then_81 =
        "// 这是含 81 与 72 等数字的注释行，必须被跳过\n"
        "530070000600195000098000060800060003400803001700020006060000280000419005000080079\n";

    Sudoku e1, e2, e3;
    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&e3, 0, sizeof(e3));
    CHECK(sudoku_from_text(p81, &e1), "expected parse");

    FILE *fp = fopen("data/_t1.ss", "w");
    fputs(ss_style, fp);
    fclose(fp);
    fp = fopen("data/_t2.txt", "w");
    fputs(comment_then_81, fp);
    fclose(fp);

    CHECK(sudoku_load("data/_t1.ss", &e2) == 1, "load .ss style");
    CHECK(sudoku_load("data/_t2.txt", &e3) == 1, "load with comment line");
    int same12 = memcmp(e1.grid, e2.grid, sizeof(e1.grid)) == 0;
    int same13 = memcmp(e1.grid, e3.grid, sizeof(e1.grid)) == 0;
    CHECK(same12, ".ss style equals expected");
    CHECK(same13, "comment-skipping equals expected");

    remove("data/_t1.ss");
    remove("data/_t2.txt");
}

static void test_generation(void)
{
    /* 普通数独：多种子生成，检查唯一解与题目正确性 */
    for (unsigned int seed = 1; seed <= 3; seed++) {
        Sudoku puzzle, solution, resolved;
        CHECK(sudoku_generate(&puzzle, &solution, 0, 0, seed) == 1,
              "normal generation ok");
        int g = sudoku_filled(&puzzle);
        CHECK(g >= 17 && g <= 45, "normal givens count in range");
        CHECK(sudoku_valid(&puzzle) == 1, "normal generated puzzle valid");
        CHECK(sudoku_has_unique_solution(&puzzle, 0) == 1,
              "normal puzzle unique");
        CHECK(solve_grid(&puzzle, 0, &resolved) == 1, "normal puzzle solvable");
        if (g >= 17 && g <= 45) {
            CHECK(memcmp(resolved.grid, solution.grid, sizeof(solution.grid)) == 0,
                  "normal solve matches generated solution");
        }
    }

    /* 星形数独：生成结果必须同时满足普通与星形约束且唯一 */
    for (unsigned int seed = 1; seed <= 2; seed++) {
        Sudoku puzzle, solution, resolved;
        CHECK(sudoku_generate(&puzzle, &solution, 1, 0, seed) == 1,
              "star generation ok");
        CHECK(sudoku_valid(&puzzle) == 1, "star generated puzzle normal-valid");
        CHECK(star_sudoku_valid(&puzzle) == 1,
              "star generated puzzle star-valid");
        CHECK(sudoku_has_unique_solution(&puzzle, 1) == 1,
              "star puzzle unique");
        CHECK(solve_grid(&puzzle, 1, &resolved) == 1, "star puzzle solvable");
        CHECK(star_sudoku_valid(&resolved) == 1,
              "star solution satisfies star region");
        CHECK(memcmp(resolved.grid, solution.grid, sizeof(solution.grid)) == 0,
              "star solve matches generated solution");
    }
}

static void test_game_helpers(void)
{
    /* 冲突检测：找一行至少两个空格，用该行未出现的数字填两个空格 */
    Sudoku p, b;
    CHECK(sudoku_generate(&p, &b, 0, 0, 11) == 1, "gen for move test");
    b = p;
    int ok = 0;
    for (int r = 0; r < 9 && !ok; r++) {
        int e1 = -1, e2 = -1;
        for (int c = 0; c < 9; c++) {
            if (b.grid[r][c] == 0) {
                if (e1 < 0) {
                    e1 = c;
                } else if (e2 < 0) {
                    e2 = c;
                    break;
                }
            }
        }
        if (e2 < 0) {
            continue;
        }
        for (int d = 1; d <= 9; d++) {
            if (sudoku_game_move_ok(&b, r + 1, e1 + 1, d, 0) == 1) {
                Sudoku conflict = b;
                conflict.grid[r][e1] = d;
                conflict.grid[r][e2] = d;
                CHECK(sudoku_game_move_ok(
                          &conflict, r + 1, e1 + 1, d, 0) == 0,
                      "row duplicate rejected");
                CHECK(sudoku_game_move_ok(&b, r + 1, e1 + 1, d, 0) == 1,
                      "legal fill into empty cell allowed");
                ok = 1;
                break;
            }
        }
    }
    CHECK(ok == 1, "found row for conflict test");
    CHECK(sudoku_game_move_ok(&b, 1, 1, 0, 0) == 1, "erase allowed");

    /* 普通：不断取 DPLL 提示填满，应还原唯一解 */
    Sudoku puzzle, solution, board;
    CHECK(sudoku_generate(&puzzle, &solution, 0, 0, 12) == 1,
          "gen normal for hint loop");
    board = puzzle;
    int steps = 0, res, hr, hc, hn;
    while ((res = sudoku_game_hint(&board, 0, &hr, &hc, &hn)) == 1 &&
           steps < 100) {
        CHECK(board.grid[hr - 1][hc - 1] == 0, "hint targets empty cell");
        CHECK(sudoku_game_move_ok(&board, hr, hc, hn, 0) == 1,
              "hint value is legal");
        board.grid[hr - 1][hc - 1] = hn;
        steps++;
    }
    CHECK(res != -1, "hint no internal error");
    CHECK(sudoku_filled(&board) == 81, "hint loop fills board");
    CHECK(memcmp(board.grid, solution.grid, sizeof(solution.grid)) == 0,
          "hint path equals unique solution");

    /* 星形：同样验证提示路径得到星形合法唯一解 */
    Sudoku sp, ss, sb;
    CHECK(sudoku_generate(&sp, &ss, 1, 0, 13) == 1, "gen star for hint loop");
    sb = sp;
    steps = 0;
    while ((res = sudoku_game_hint(&sb, 1, &hr, &hc, &hn)) == 1 &&
           steps < 100) {
        sb.grid[hr - 1][hc - 1] = hn;
        steps++;
    }
    CHECK(res != -1, "star hint no internal error");
    CHECK(sudoku_filled(&sb) == 81, "star hint loop fills board");
    CHECK(star_sudoku_valid(&sb) == 1, "star hint path star-valid");
    CHECK(memcmp(sb.grid, ss.grid, sizeof(ss.grid)) == 0,
          "star hint path equals unique solution");

    /* 已填满时 hint 返回 0 */
    CHECK(sudoku_game_hint(&ss, 1, &hr, &hc, &hn) == 0,
          "complete board gives no hint");
    CHECK(sudoku_game_solvable(&sp, 1) == 1, "generated star solvable");
}

int main(void)
{
    test_encoding();
    test_clause_counts();
    test_normal_solve();
    test_star_solve();
    test_loader();
    test_generation();
    test_game_helpers();

    printf("checks=%d fails=%d\n", g_checks, g_fails);
    return g_fails == 0 ? 0 : 1;
}
