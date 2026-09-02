#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dpll.h"

/* 文字 <-> 编号：id = 2*|L| + (L<0 ? 1 : 0)，id^1 即取反 */
static int lit_id(int L)
{
    return 2 * abs(L) + (L < 0 ? 1 : 0);
}

/* =====================================================================
 * 基准（未优化）DPLL
 *   - 选取第一个未赋值变元
 *   - 每轮单元传播扫描所有子句
 *   - 每个分支复制整个赋值数组
 * 用于测量优化前的执行时间 t。
 * ===================================================================== */

int find_unit_literal(const Clause *clause, const int *assignment)
{
    int unassigned_count = 0;
    int unit_literal = 0;

    for (int i = 0; i < clause->size; i++) {
        int literal = clause->literals[i];
        int variable = abs(literal);

        if ((literal > 0 && assignment[variable] == TRUE_VALUE) ||
            (literal < 0 && assignment[variable] == FALSE_VALUE)) {
            return 0;                     /* 已满足，不是单元子句 */
        }
        if (assignment[variable] == UNASSIGNED) {
            unassigned_count++;
            unit_literal = literal;
        }
    }

    if (unassigned_count == 1) {
        return unit_literal;
    }
    return 0;
}

int unit_propagation(Formula *formula, int *assignment)
{
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < formula->clause_count; i++) {
            Clause *clause = &formula->clauses[i];
            int literal = find_unit_literal(clause, assignment);
            if (literal != 0) {
                int variable = abs(literal);
                if (assignment[variable] == UNASSIGNED) {
                    assignment[variable] =
                        literal > 0 ? TRUE_VALUE : FALSE_VALUE;
                    changed = 1;
                }
            }
        }
        if (is_formula_satisfied(formula, assignment) == FORMULA_FALSE) {
            return 0;
        }
    }
    return 1;
}

static int choose_variable_naive(const Formula *formula,
                                 const int *assignment)
{
    for (int i = 1; i <= formula->variable_count; i++) {
        if (assignment[i] == UNASSIGNED) {
            return i;
        }
    }
    return 0;
}

static int dpll_naive(Formula *formula, int *assignment)
{
    if (!unit_propagation(formula, assignment)) {
        return 0;
    }

    FormulaStatus status = is_formula_satisfied(formula, assignment);
    if (status == FORMULA_TRUE) {
        return 1;
    }
    if (status == FORMULA_FALSE) {
        return 0;
    }

    int variable = choose_variable_naive(formula, assignment);
    if (variable == 0) {
        return 0;
    }

    int *backup = (int *)malloc(sizeof(int) * (formula->variable_count + 1));
    if (backup == NULL) {
        return 0;
    }
    for (int i = 1; i <= formula->variable_count; i++) {
        backup[i] = assignment[i];
    }

    assignment[variable] = TRUE_VALUE;
    if (dpll_naive(formula, assignment)) {
        free(backup);
        return 1;
    }

    for (int i = 1; i <= formula->variable_count; i++) {
        assignment[i] = backup[i];
    }
    assignment[variable] = FALSE_VALUE;
    if (dpll_naive(formula, assignment)) {
        free(backup);
        return 1;
    }

    for (int i = 1; i <= formula->variable_count; i++) {
        assignment[i] = backup[i];
    }
    free(backup);
    return 0;
}

/* =====================================================================
 * 优化 DPLL
 *   - 出现表（occurrence list）：单元传播只需访问受赋值影响的子句，
 *     避免每轮全量扫描所有子句（存储结构优化）
 *   - 子句满足计数 + trail：回溯时只撤销本层改动，避免每层复制整个赋值数组
 * ===================================================================== */

typedef struct {
    Formula *f;
    int n, m;
    int *assign;              /* 1..n: UNASSIGNED/FALSE/TRUE */
    int *occ_cnt;             /* 每个文字出现次数 */
    int *occ_start;           /* 每个文字在 occ_clauses 中的起始位置 */
    int *occ_clauses;         /* 每个文字对应的子句下标列表（扁平存放） */
    int *clause_sat;          /* 1 = 该子句已被满足 */
    int sat_count;            /* 已满足子句数 */
    int *trail_var;           /* 赋值轨迹（用于回溯） */
    int trail_len;
    int *sat_undo;            /* 记录“由未满足变为满足”的子句 */
    int sat_undo_len;
    int *queue;               /* 单元文字队列 */
    int qhead, qtail;
} Solver;

/* 将一个文字赋值为真/假，并处理由此产生的满足与单元/冲突 */
static int assign_lit(Solver *s, int lit)
{
    int v = abs(lit);
    int val = lit > 0 ? TRUE_VALUE : FALSE_VALUE;

    if (s->assign[v] == val) {
        return 1;
    }
    if (s->assign[v] != UNASSIGNED) {
        return 0;                                   /* 冲突 */
    }

    s->assign[v] = val;
    s->trail_var[s->trail_len++] = v;

    int t_id = lit_id(lit);
    int n_id = t_id ^ 1;

    /* 该文字为真 → 相关子句被满足 */
    for (int i = s->occ_start[t_id]; i < s->occ_start[t_id + 1]; i++) {
        int c = s->occ_clauses[i];
        if (!s->clause_sat[c]) {
            s->clause_sat[c] = 1;
            s->sat_count++;
            s->sat_undo[s->sat_undo_len++] = c;
        }
    }

    /* 该文字为假 → 相关子句可能成为单元或空子句 */
    for (int i = s->occ_start[n_id]; i < s->occ_start[n_id + 1]; i++) {
        int c = s->occ_clauses[i];
        if (s->clause_sat[c]) {
            continue;                               /* 已满足则无需处理 */
        }
        Clause *cl = &s->f->clauses[c];
        int unassigned = 0, unit_lit = 0, false_cnt = 0;
        for (int j = 0; j < cl->size; j++) {
            int L = cl->literals[j];
            int vv = abs(L);
            int av = s->assign[vv];
            if (av == UNASSIGNED) {
                unassigned++;
                unit_lit = L;
            } else if ((L > 0 && av == FALSE_VALUE) ||
                       (L < 0 && av == TRUE_VALUE)) {
                false_cnt++;
            }
        }
        if (false_cnt == cl->size) {
            return 0;                               /* 空子句，冲突 */
        }
        if (unassigned == 1) {
            s->queue[s->qtail++] = unit_lit;        /* 单元文字 */
        }
    }
    return 1;
}

static int run_queue(Solver *s)
{
    while (s->qhead < s->qtail) {
        int lit = s->queue[s->qhead++];
        if (!assign_lit(s, lit)) {
            return 0;
        }
    }
    return 1;
}

/* 单元传播：先播种当前的所有单元子句，再处理级联。
 * 该函数只在求解开始时调用一次；之后的单元传播由 assign_lit
 * 中的出现表逻辑驱动，无需再全量扫描所有子句。 */
static int seed_initial(Solver *s)
{
    s->qhead = s->qtail = 0;

    for (int c = 0; c < s->m; c++) {
        if (s->clause_sat[c]) {
            continue;
        }
        Clause *cl = &s->f->clauses[c];
        int unassigned = 0, unit_lit = 0, false_cnt = 0;
        for (int j = 0; j < cl->size; j++) {
            int L = cl->literals[j];
            int vv = abs(L);
            int av = s->assign[vv];
            if (av == UNASSIGNED) {
                unassigned++;
                unit_lit = L;
            } else if ((L > 0 && av == FALSE_VALUE) ||
                       (L < 0 && av == TRUE_VALUE)) {
                false_cnt++;
            }
        }
        if (false_cnt == cl->size) {
            return 0;                               /* 空子句，冲突 */
        }
        if (unassigned == 1) {
            s->queue[s->qtail++] = unit_lit;        /* 单元子句 */
        }
    }
    return run_queue(s);
}

/* 回溯到指定深度 */
static void undo_to(Solver *s, int tr_len, int su_len)
{
    while (s->trail_len > tr_len) {
        int v = s->trail_var[--s->trail_len];
        s->assign[v] = UNASSIGNED;
    }
    while (s->sat_undo_len > su_len) {
        int c = s->sat_undo[--s->sat_undo_len];
        s->clause_sat[c] = 0;
        s->sat_count--;
    }
}

/* 分支变元选取：取第一个未赋值变元（保持与基准相同的搜索顺序，
 * 以便清晰对比出“存储结构/传播”优化带来的收益） */
static int choose_literal_optimized(Solver *s)
{
    for (int v = 1; v <= s->n; v++) {
        if (s->assign[v] == UNASSIGNED) {
            return v;
        }
    }
    return 0;
}

static int dpll_core(Solver *s)
{
    if (s->sat_count == s->m) {
        return 1;                                   /* 全部子句满足 */
    }

    int lit = choose_literal_optimized(s);
    if (lit == 0) {
        return 0;
    }

    int tr = s->trail_len;
    int su = s->sat_undo_len;

    s->qhead = s->qtail = 0;
    if (assign_lit(s, lit) && run_queue(s)) {
        if (dpll_core(s)) {
            return 1;
        }
    }
    undo_to(s, tr, su);

    s->qhead = s->qtail = 0;
    if (assign_lit(s, -lit) && run_queue(s)) {
        if (dpll_core(s)) {
            return 1;
        }
    }
    undo_to(s, tr, su);

    return 0;
}

static int solve_optimized(Formula *f, int *assignment)
{
    int n = f->variable_count;
    int m = f->clause_count;

    Solver *s = (Solver *)calloc(1, sizeof(Solver));
    if (s == NULL) {
        return 0;
    }
    s->f = f;
    s->n = n;
    s->m = m;
    s->assign = (int *)malloc(sizeof(int) * (n + 1));
    s->occ_cnt = (int *)calloc((size_t)(2 * n + 2), sizeof(int));
    s->occ_start = (int *)malloc(sizeof(int) * (2 * n + 3));
    s->clause_sat = (int *)calloc((size_t)(m > 0 ? m : 1), sizeof(int));
    s->trail_var = (int *)malloc(sizeof(int) * (n + 1));
    s->sat_undo = (int *)malloc(sizeof(int) * (m > 0 ? m + 1 : 1));
    s->queue = (int *)malloc(sizeof(int) * (n + m + 8));

    if (s->assign == NULL || s->occ_cnt == NULL || s->occ_start == NULL ||
        s->clause_sat == NULL || s->trail_var == NULL || s->sat_undo == NULL ||
        s->queue == NULL) {
        /* 内存不足：统一释放 */
        free(s->assign); free(s->occ_cnt); free(s->occ_start);
        free(s->clause_sat); free(s->trail_var); free(s->sat_undo);
        free(s->queue);
        free(s);
        return 0;
    }

    for (int v = 0; v <= n; v++) {
        s->assign[v] = UNASSIGNED;
    }

    long total = 0;
    for (int c = 0; c < m; c++) {
        Clause *cl = &f->clauses[c];
        for (int j = 0; j < cl->size; j++) {
            s->occ_cnt[lit_id(cl->literals[j])]++;
            total++;
        }
    }

    s->occ_start[0] = 0;
    for (int i = 1; i <= 2 * n + 2; i++) {
        s->occ_start[i] = s->occ_start[i - 1] + s->occ_cnt[i - 1];
    }
    s->occ_clauses =
        (int *)malloc(sizeof(int) * (size_t)(total > 0 ? total : 1));
    if (s->occ_clauses == NULL) {
        free(s->assign); free(s->occ_cnt); free(s->occ_start);
        free(s->clause_sat); free(s->trail_var); free(s->sat_undo);
        free(s->queue);
        free(s);
        return 0;
    }

    int *cur = (int *)malloc(sizeof(int) * (2 * n + 3));
    for (int i = 0; i <= 2 * n + 2; i++) {
        cur[i] = s->occ_start[i];
    }
    for (int c = 0; c < m; c++) {
        Clause *cl = &f->clauses[c];
        for (int j = 0; j < cl->size; j++) {
            int id = lit_id(cl->literals[j]);
            s->occ_clauses[cur[id]++] = c;
        }
    }
    free(cur);

    int result;
    if (!seed_initial(s)) {
        result = 0;                                 /* 初始冲突（空子句等） */
    } else {
        result = dpll_core(s);
    }

    for (int v = 1; v <= n; v++) {
        assignment[v] = s->assign[v];
    }

    free(s->occ_cnt); free(s->occ_start); free(s->occ_clauses);
    free(s->clause_sat); free(s->trail_var); free(s->sat_undo);
    free(s->assign); free(s->queue);
    free(s);
    return result;
}

int dpll_mode(Formula *formula, int *assignment, int mode)
{
    if (formula == NULL || assignment == NULL) {
        return 0;
    }
    int n = formula->variable_count;
    for (int i = 1; i <= n; i++) {
        assignment[i] = UNASSIGNED;
    }

    if (mode == DPLL_MODE_OPTIMIZED) {
        return solve_optimized(formula, assignment);
    }
    return dpll_naive(formula, assignment);
}

int dpll(Formula *formula, int *assignment)
{
    return dpll_mode(formula, assignment, DPLL_MODE_OPTIMIZED);
}
