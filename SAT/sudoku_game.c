#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku_game.h"
#include "sudoku_cnf.h"
#include "star_sudoku.h"
#include "sudoku_gen.h"
#include "dpll.h"

/* 与 sudoku.c 中相同的区域取值判断，但允许“将要填的 num”参与检查 */
static int line_conflict_with(const Sudoku *board, int r0, int c0,
                              int num)
{
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        if (board->grid[r0][i] == num && i != c0) {
            return 1;
        }
        if (board->grid[i][c0] == num && i != r0) {
            return 1;
        }
    }
    int br = (r0 / SUDOKU_BOX) * SUDOKU_BOX;
    int bc = (c0 / SUDOKU_BOX) * SUDOKU_BOX;
    for (int i = 0; i < SUDOKU_BOX; i++) {
        for (int j = 0; j < SUDOKU_BOX; j++) {
            int r = br + i;
            int c = bc + j;
            if ((r != r0 || c != c0) && board->grid[r][c] == num) {
                return 1;
            }
        }
    }
    return 0;
}

int sudoku_game_move_ok(const Sudoku *board, int row, int col, int num,
                        int with_star)
{
    if (board == NULL || row < 1 || row > 9 || col < 1 || col > 9 ||
        num < 0 || num > 9) {
        return 0;
    }
    if (num == 0) {
        return 1;
    }
    if (line_conflict_with(board, row - 1, col - 1, num)) {
        return 0;
    }
    if (with_star) {
        for (int i = 0; i < SUDOKU_SIZE; i++) {
            int sr = star_cells[i][0];
            int sc = star_cells[i][1];
            if ((sr != row || sc != col) && board->grid[sr - 1][sc - 1] == num) {
                return 0;
            }
        }
    }
    return 1;
}

static int board_legal(const Sudoku *board, int with_star)
{
    return sudoku_valid(board) && (!with_star || star_sudoku_valid(board));
}

static Formula *board_cnf(const Sudoku *board, int with_star)
{
    if (!board_legal(board, with_star)) {
        return NULL;
    }
    return with_star ? star_sudoku_make_cnf(board)
                     : sudoku_make_cnf(board);
}

/* 求解当前盘面并还原；返回 1 成功 */
static int solve_board(const Sudoku *board, int with_star, Sudoku *out)
{
    Formula *f = board_cnf(board, with_star);
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
    int ok = dpll(f, a) && sudoku_from_assignment(a, out);
    free(a);
    free_formula(f);
    return ok ? 1 : 0;
}

int sudoku_game_solvable(const Sudoku *board, int with_star)
{
    Sudoku tmp;
    int r = solve_board(board, with_star, &tmp);
    if (r < 0) {
        return -1;
    }
    return r;                       /* 1=可解 0=不可解 */
}

int sudoku_game_hint(const Sudoku *board, int with_star,
                     int *row, int *col, int *num)
{
    if (board == NULL || row == NULL || col == NULL || num == NULL) {
        return -1;
    }
    if (sudoku_filled(board) == SUDOKU_SIZE * SUDOKU_SIZE) {
        return 0;                   /* 已填满，无需提示 */
    }

    Sudoku sol;
    int r = solve_board(board, with_star, &sol);
    if (r <= 0) {
        return r;                   /* 0 无解；-1 错误 */
    }
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (board->grid[i][j] == 0) {
                *row = i + 1;
                *col = j + 1;
                *num = sol.grid[i][j];
                return 1;
            }
        }
    }
    return 0;
}

/* 带行列号的棋盘显示 */
static void print_board(const Sudoku *board)
{
    printf("    1 2 3   4 5 6   7 8 9\n");
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        printf("%d  ", r + 1);
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            if (board->grid[r][c] == 0) {
                printf(". ");
            } else {
                printf("%d ", board->grid[r][c]);
            }
            if (c == 2 || c == 5) {
                printf("| ");
            }
        }
        printf("\n");
        if (r == 2 || r == 5) {
            printf("   ------+-------+------\n");
        }
    }
}

int sudoku_play(int with_star, unsigned int seed)
{
    Sudoku puzzle, answer;
    if (!sudoku_generate(&puzzle, &answer, with_star, 0, seed)) {
        printf("Failed to generate a puzzle.\n");
        return 1;
    }

    Sudoku board = puzzle;
    int locked[SUDOKU_SIZE][SUDOKU_SIZE];
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            locked[r][c] = puzzle.grid[r][c] != 0;
        }
    }

    printf("\n== %s Sudoku (givens=%d) ==\n",
           with_star ? "Star" : "Normal", sudoku_filled(&puzzle));
    printf("Commands:\n");
    printf("  fill <row> <col> <num>   put num at (row,col)\n");
    printf("  erase <row> <col>        clear (row,col)\n");
    printf("  hint                    ask DPLL for one hint\n");
    printf("  check                   is current board still solvable?\n");
    printf("  answer                  show the solution\n");
    printf("  quit                    back to menu\n");

    char cmd[32];
    while (1) {
        printf("\n");
        print_board(&board);
        printf("> ");
        if (scanf("%31s", cmd) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { /* skip */ }
            continue;
        }

        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            break;
        } else if (strcmp(cmd, "hint") == 0 || strcmp(cmd, "h") == 0) {
            int r, c, n;
            int res = sudoku_game_hint(&board, with_star, &r, &c, &n);
            if (res == 1) {
                printf("Hint: (%d,%d) = %d\n", r, c, n);
            } else if (res == 0) {
                printf("Board is already complete.\n");
            } else {
                printf("No solution from current state.\n");
            }
        } else if (strcmp(cmd, "check") == 0) {
            int s = sudoku_game_solvable(&board, with_star);
            printf(s == 1 ? "Board is solvable.\n"
                          : (s == 0 ? "Board is NOT solvable.\n"
                                    : "Board contains conflicts.\n"));
        } else if (strcmp(cmd, "answer") == 0) {
            Sudoku sol;
            int s = solve_board(&board, with_star, &sol);
            if (s == 1) {
                printf("Solution:\n");
                sudoku_print(&sol);
            } else {
                printf("No solution from current state.\n");
            }
        } else if (strcmp(cmd, "fill") == 0 || strcmp(cmd, "erase") == 0) {
            int r, c, n = 0;
            if (scanf("%d %d", &r, &c) != 2) {
                printf("Bad arguments.\n");
                continue;
            }
            if (strcmp(cmd, "fill") == 0) {
                if (scanf("%d", &n) != 1) {
                    printf("Bad arguments.\n");
                    continue;
                }
            }
            if (r < 1 || r > 9 || c < 1 || c > 9 ||
                (strcmp(cmd, "fill") == 0 && (n < 1 || n > 9))) {
                printf("Coordinates/digit out of range.\n");
                continue;
            }
            if (locked[r - 1][c - 1]) {
                printf("(%d,%d) is a given, cannot modify.\n", r, c);
                continue;
            }

            Sudoku trial = board;
            trial.grid[r - 1][c - 1] = n;       /* erase 时 n=0 */
            if (!sudoku_game_move_ok(&trial, r, c, n, with_star)) {
                printf("Conflicts with existing numbers.\n");
                continue;
            }
            if (n != 0 && sudoku_game_solvable(&trial, with_star) != 1) {
                printf("That move makes the puzzle unsolvable.\n");
                continue;
            }
            board = trial;

            if (n != 0 && sudoku_filled(&board) == 81 &&
                sudoku_valid(&board) &&
                (!with_star || star_sudoku_valid(&board))) {
                printf("\nCongratulations, you solved it!\n");
                print_board(&board);
                break;
            }
        } else {
            printf("Unknown command.\n");
        }
    }
    return 0;
}
