#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnf.h"

/* 读取 DIMACS 格式的 CNF 文件。
 * 支持：
 *   - 'c' 开头的注释行、空行（可出现在任意位置）
 *   - 'p cnf <变量数> <子句数>' 头部行
 *   - 任意长度、任意数量的字句，0 作为子句结束标记
 * 失败返回 NULL。
 */
Formula *load_cnf(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    Formula *formula = (Formula *)malloc(sizeof(Formula));
    if (formula == NULL) {
        fclose(file);
        return NULL;
    }

    formula->variable_count = 0;
    formula->clause_count = 0;
    formula->clauses = NULL;

    char line[4096];
    int got_header = 0;

    /* 扫描头部：跳过注释与空行，找到 'p cnf' 行 */
    while (fgets(line, sizeof(line), file) != NULL) {
        char *p = line;
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p == '\0' || *p == '\n' || *p == '\r') {
            continue;                                   /* 空行 */
        }
        if (*p == 'c') {
            continue;                                   /* 注释 */
        }
        if (*p == 'p') {
            int vars = 0, clauses = 0;
            if (sscanf(p, "p cnf %d %d", &vars, &clauses) == 2 &&
                vars >= 0 && clauses >= 0) {
                formula->variable_count = vars;
                formula->clause_count = clauses;
                got_header = 1;
            }
            break;
        }
        /* 其余行（如超长注释的续行）忽略 */
    }

    if (!got_header) {
        fclose(file);
        free(formula);
        return NULL;
    }

    if (formula->clause_count > 0) {
        formula->clauses =
            (Clause *)calloc((size_t)formula->clause_count, sizeof(Clause));
        if (formula->clauses == NULL) {
            fclose(file);
            free(formula);
            return NULL;
        }
    }

    /* 逐子句读取；使用字符流以便正确处理穿插的注释/空行和超长子句 */
    int ci = 0;
    while (ci < formula->clause_count) {
        int ch = fgetc(file);
        if (ch == EOF) {
            break;
        }
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            continue;
        }
        if (ch == 'c') {                                /* 行注释 */
            while ((ch = fgetc(file)) != EOF && ch != '\n') {
                /* skip */
            }
            continue;
        }
        if (ch == 'p') {                                /* 多余头部，忽略 */
            while ((ch = fgetc(file)) != EOF && ch != '\n') {
                /* skip */
            }
            continue;
        }
        ungetc(ch, file);                               /* 回到子句首文字 */

        Clause *cl = &formula->clauses[ci];
        cl->literals = NULL;
        cl->size = 0;
        int cap = 0, val;

        while (fscanf(file, "%d", &val) == 1 && val != 0) {
            if (cl->size == cap) {
                cap = cap == 0 ? 8 : cap * 2;
                int *tmp = (int *)realloc(cl->literals,
                                          (size_t)cap * sizeof(int));
                if (tmp == NULL) {
                    fclose(file);
                    for (int k = 0; k <= ci; k++) {
                        free(formula->clauses[k].literals);
                    }
                    free(formula->clauses);
                    free(formula);
                    return NULL;
                }
                cl->literals = tmp;
            }
            cl->literals[cl->size++] = val;
        }
        ci++;
    }

    /* 以实际成功读取的子句数为准 */
    formula->clause_count = ci;

    fclose(file);
    return formula;
}

/* 打印整个公式（用于解析正确性的目测验证） */
void print_formula(const Formula *formula)
{
    if (formula == NULL) {
        return;
    }
    printf("Variables: %d\n", formula->variable_count);
    printf("Clauses: %d\n", formula->clause_count);
    for (int i = 0; i < formula->clause_count; i++) {
        printf("Clause %d: ", i + 1);
        for (int j = 0; j < formula->clauses[i].size; j++) {
            printf("%d ", formula->clauses[i].literals[j]);
        }
        printf("\n");
    }
}

/* 释放公式占用的全部内存 */
void free_formula(Formula *formula)
{
    if (formula == NULL) {
        return;
    }
    for (int i = 0; i < formula->clause_count; i++) {
        free(formula->clauses[i].literals);
    }
    free(formula->clauses);
    free(formula);
}

/* 判断子句在当前赋值下的状态 */
ClauseStatus is_clause_satisfied(const Clause *clause, const int *assignment)
{
    int has_unassigned = 0;
    for (int i = 0; i < clause->size; i++) {
        int literal = clause->literals[i];
        if (literal > 0) {
            if (assignment[literal] == TRUE_VALUE) {
                return CLAUSE_TRUE;
            } else if (assignment[literal] == UNASSIGNED) {
                has_unassigned = 1;
            }
        } else {
            int variable = -literal;
            if (assignment[variable] == FALSE_VALUE) {
                return CLAUSE_TRUE;
            } else if (assignment[variable] == UNASSIGNED) {
                has_unassigned = 1;
            }
        }
    }
    if (has_unassigned) {
        return CLAUSE_UNKNOWN;
    }
    return CLAUSE_FALSE;
}

/* 判断公式在当前赋值下的状态 */
FormulaStatus is_formula_satisfied(const Formula *formula,
                                   const int *assignment)
{
    int has_unknown = 0;
    for (int i = 0; i < formula->clause_count; i++) {
        ClauseStatus status =
            is_clause_satisfied(&formula->clauses[i], assignment);
        if (status == CLAUSE_FALSE) {
            return FORMULA_FALSE;
        } else if (status == CLAUSE_UNKNOWN) {
            has_unknown = 1;
        }
    }
    if (has_unknown) {
        return FORMULA_UNKNOWN;
    }
    return FORMULA_TRUE;
}
