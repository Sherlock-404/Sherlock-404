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
     free_formula(formula);

     return 0;
 }
