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
#include <stdlib.h>

#include "cnf.h"
#include "dpll.h"

int main(void)
{
    Formula *formula = load_cnf("data/test.cnf");

    if (formula == NULL) {
        printf("Failed to load CNF file.\n");
        return 1;
    }

    print_formula(formula);

    int *assignment =
        malloc(sizeof(int) *
               (formula->variable_count + 1));

    if (assignment == NULL) {
        free_formula(formula);
        return 1;
    }

    for (int i = 1;
         i <= formula->variable_count;
         i++) {

        assignment[i] = UNASSIGNED;
    }

    printf("\nSolving...\n");

    if (dpll(formula, assignment)) {

        printf("\nSAT!\n");

        for (int i = 1;
             i <= formula->variable_count;
             i++) {

            printf("x%d = %d\n",
                   i,
                   assignment[i]);
        }

    } else {

        printf("\nUNSAT!\n");
    }

    free(assignment);
    free_formula(formula);

    return 0;
}