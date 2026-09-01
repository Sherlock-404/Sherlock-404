//   #include <stdio.h>
//   #include "cnf.h"

//  int main(void)
// {
//       Formula *formula = load_cnf("data/test.cnf");

//       if (formula == NULL) {
//          printf("Failed to load CNF file.\n");
//          return 1;
//     }
//      print_formula(formula);
//      free_formula(formula);

//      return 0;
//  }
#include <stdio.h>
#include "cnf.h"

int main(void)
{
    Formula *formula = load_cnf("data/test.cnf");

    if (formula == NULL) {
        printf("Failed to load CNF file.\n");
        return 1;
    }

    print_formula(formula);

    int assignment[4];

    /*
     * 情况1：全部未赋值
     */
    assignment[1] = UNASSIGNED;
    assignment[2] = UNASSIGNED;
    assignment[3] = UNASSIGNED;

    printf("\nCase 1: ");
    
    FormulaStatus result =
        is_formula_satisfied(formula, assignment);

    if (result == FORMULA_TRUE)
        printf("TRUE\n");
    else if (result == FORMULA_FALSE)
        printf("FALSE\n");
    else
        printf("UNKNOWN\n");


    /*
     * 情况2：x1=false, x2=true, x3=false
     */
    assignment[1] = FALSE_VALUE;
    assignment[2] = TRUE_VALUE;
    assignment[3] = FALSE_VALUE;

    printf("Case 2: ");

    result =
        is_formula_satisfied(formula, assignment);

    if (result == FORMULA_TRUE)
        printf("TRUE\n");
    else if (result == FORMULA_FALSE)
        printf("FALSE\n");
    else
        printf("UNKNOWN\n");


    free_formula(formula);

    return 0;
}