#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include "cnf.h"
#include "dpll.h"

/* 高精度毫秒计时（便于优化率比较；.res 文件中仍按 time.h 的 clock 计时） */
static double now_ms(void)
{
#ifdef _WIN32
    static LARGE_INTEGER freq = {0}, start = {0};
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
    }
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    return (double)((pc.QuadPart - start.QuadPart) * 1000.0) /
           (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
#endif
}

/* 按规范把结果写到与算例同名的 .res 文件
 * s 求解结果   (1=满足, 0=不满足)
 * v 赋值序列   (满足时：每个变元的赋值，-i 表假，i 表真)
 * t 执行时间   (毫秒)
 */
static void write_res_file(const char *cnf_path, int result,
                           const int *assignment, double ms, int n)
{
    char base[1024];
    snprintf(base, sizeof(base), "%s", cnf_path);
    char *dot = strrchr(base, '.');
    if (dot != NULL) {
        *dot = '\0';
    }

    char res[1100];
    snprintf(res, sizeof(res), "%s.res", base);

    FILE *fp = fopen(res, "w");
    if (fp == NULL) {
        printf("Failed to write result file: %s\n", res);
        return;
    }

    fprintf(fp, "s %d\n", result ? 1 : 0);
    if (result) {
        fprintf(fp, "v");
        for (int i = 1; i <= n; i++) {
            if (assignment[i] == TRUE_VALUE) {
                fprintf(fp, " %d", i);
            } else {
                fprintf(fp, " %d", -i);   /* 未赋值或无关键字：视为假 */
            }
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "t %.0f\n", ms);
    fclose(fp);

    printf("Result saved to %s\n", res);
}

/* 对内存中的公式求解并输出 .res（使用 time.h 的 clock 计时） */
static void run_solve(Formula *formula, const char *path, int mode)
{
    int n = formula->variable_count;
    int *assignment = (int *)malloc(sizeof(int) * (n + 1));
    if (assignment == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }
    for (int i = 0; i <= n; i++) {
        assignment[i] = UNASSIGNED;
    }

    clock_t start = clock();
    int result = dpll_mode(formula, assignment, mode);
    clock_t end = clock();
    double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    printf("Result: %s\n", result ? "SAT" : "UNSAT");
    printf("DPLL Time: %.3f ms\n", ms);
    write_res_file(path, result, assignment, ms, n);

    free(assignment);
}

static void solve_file(const char *path, int mode)
{
    Formula *formula = load_cnf(path);
    if (formula == NULL) {
        printf("Failed to load CNF file: %s\n", path);
        return;
    }
    printf("Loaded %s (vars=%d clauses=%d)\n",
           path, formula->variable_count, formula->clause_count);
    run_solve(formula, path, mode);
    free_formula(formula);
}

/* 同一算例下，基准与优化的对比，给出优化率 */
static void bench_file(const char *path)
{
    Formula *formula = load_cnf(path);
    if (formula == NULL) {
        printf("Failed to load CNF file: %s\n", path);
        return;
    }

    int n = formula->variable_count;
    int *a0 = (int *)malloc(sizeof(int) * (n + 1));
    int *a1 = (int *)malloc(sizeof(int) * (n + 1));
    for (int i = 0; i <= n; i++) {
        a0[i] = a1[i] = UNASSIGNED;
    }

    double t = now_ms();
    int r0 = dpll_mode(formula, a0, DPLL_MODE_NAIVE);
    t = now_ms() - t;

    double t0 = now_ms();
    int r1 = dpll_mode(formula, a1, DPLL_MODE_OPTIMIZED);
    t0 = now_ms() - t0;

    printf("\nFile: %s  vars=%d clauses=%d\n",
           path, n, formula->clause_count);
    printf("Baseline (naive)  : %s  %8.3f ms\n", r0 ? "SAT" : "UNSAT", t);
    printf("Optimized         : %s  %8.3f ms\n", r1 ? "SAT" : "UNSAT", t0);
    if (r0 != r1) {
        printf("WARNING: baseline and optimized results differ!\n");
    }
    if (t > 0.0) {
        printf("Optimization rate : %.2f%%\n", (t - t0) / t * 100.0);
    } else {
        printf("Optimization rate : n/a (baseline ~0 ms)\n");
    }

    free(a0);
    free(a1);
    free_formula(formula);
}

static void print_usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s <cnf_file> [mode]      solve one instance; mode 0/1 (default 1)\n",
           prog);
    printf("  %s --bench <cnf_file>     compare baseline vs optimized\n", prog);
    printf("  (no argument -> interactive menu)\n");
}

/* 交互式菜单 */
static void sat_menu(void)
{
    char filename[1024] = "";
    Formula *formula = NULL;
    int choice;

    while (1) {
        printf("\n");
        printf("========== SAT Solver ==========\n");
        printf("1. Load CNF file\n");
        printf("2. Print Formula (verification)\n");
        printf("3. Solve CNF\n");
        printf("4. Benchmark (baseline vs optimized)\n");
        printf("0. Back\n");
        printf("================================\n");
        printf("Please enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { /* skip */ }
            continue;
        }

        if (choice == 1) {
            printf("Please enter CNF file path: ");
            scanf("%1023s", filename);
            formula = load_cnf(filename);
            if (formula == NULL) {
                printf("Failed to load CNF file.\n");
            } else {
                printf("CNF file loaded successfully (vars=%d clauses=%d).\n",
                       formula->variable_count, formula->clause_count);
            }
        } else if (choice == 2) {
            if (formula == NULL) {
                printf("Please load a CNF file first.\n");
            } else {
                print_formula(formula);
            }
        } else if (choice == 3) {
            if (formula == NULL) {
                printf("Please load a CNF file first.\n");
            } else {
                printf("\nSolving...\n");
                run_solve(formula, filename, DPLL_MODE_OPTIMIZED);
            }
        } else if (choice == 4) {
            if (formula == NULL) {
                printf("Please load a CNF file first.\n");
            } else {
                bench_file(filename);
            }
        } else if (choice == 0) {
            break;
        } else {
            printf("Invalid choice!\n");
        }
    }

    if (formula != NULL) {
        free_formula(formula);
    }
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "--bench") == 0) {
        if (argc < 3) {
            print_usage(argv[0]);
            return 1;
        }
        bench_file(argv[2]);
        return 0;
    }

    if (argc >= 2) {
        int mode = argc >= 3 ? atoi(argv[2]) : DPLL_MODE_OPTIMIZED;
        if (mode != DPLL_MODE_NAIVE && mode != DPLL_MODE_OPTIMIZED) {
            printf("Invalid mode: %s (use 0 or 1)\n", argv[2]);
            return 1;
        }
        printf("Solve: %s (mode=%s)\n",
               argv[1], mode == DPLL_MODE_OPTIMIZED ? "optimized" : "naive");
        solve_file(argv[1], mode);
        return 0;
    }

    int choice;
    while (1) {
        printf("\n");
        printf("========== SAT Solver ==========\n");
        printf("1. SAT Solver\n");
        printf("2. Asterisk Sudoku\n");
        printf("0. Exit\n");
        printf("================================\n");
        printf("Please enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { /* skip */ }
            continue;
        }

        if (choice == 1) {
            sat_menu();
        } else if (choice == 2) {
            printf("\nAsterisk Sudoku module is under development.\n");
        } else if (choice == 0) {
            printf("\nBye!\n");
            break;
        } else {
            printf("\nInvalid choice!\n");
        }
    }

    return 0;
}
