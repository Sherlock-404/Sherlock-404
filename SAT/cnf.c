#include <stdio.h>
#include <stdlib.h>
#include "cnf.h"

//读取cnf
Formula *load_cnf(const char *filename)
{
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return NULL;
    }

    Formula *formula = malloc(sizeof(Formula));

    if (formula == NULL) {
        fclose(file);
        return NULL;
    }

    formula->variable_count = 0;
    formula->clause_count = 0;
    formula->clauses= NULL;

    char line[256];

//从字符串中读取格式化数据
    while (fgets(line, sizeof(line), file) != NULL) {

        if (line[0] == 'p') {

            sscanf(line, "p cnf %d %d",&formula->variable_count,&formula->clause_count);
            
            break;
        }
    }

//给clause分配内存
    formula->clauses = malloc(sizeof(Clause) * formula->clause_count);

            if (formula->clauses == NULL) {
                    fclose(file);
                    free(formula);
                    return NULL;    
            }


//读取每一个子句
for (int clause_index=0;clause_index<formula->clause_count;clause_index++){
    int temp[1000];
    int size=0;
    int value;
    while(fscanf(file,"%d",&value)==1 && value != 0){ 
        temp[size++]=value;
}            
formula->clauses[clause_index].literals=malloc(sizeof(int)*size);
if(formula->clauses[clause_index].literals==NULL){  
    
    fclose(file);
    free(formula->clauses);
    free(formula);
    return NULL;
}

for(int i=0;i<size;i++){
    formula->clauses[clause_index].literals[i]=temp[i];
}

 formula->clauses[clause_index].size = size;
}
    fclose(file);

    return formula;
}


//打印
void print_formula(const Formula *formula)
{
    if(formula==NULL){
        return;
    }
    printf("Variables:%d\n",formula->variable_count);
    printf("Clauses:%d\n",formula->clause_count);
    for(int i=0;i<formula->clause_count;i++){
        printf("Clause %d: ",i+1);
        for(int j=0;j<formula->clauses[i].size;j++){
            printf("%d ",formula->clauses[i].literals[j]);
        }
        printf("\n");
    }
}

//释放
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