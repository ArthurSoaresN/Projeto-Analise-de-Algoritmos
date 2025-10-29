#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CODE_LENGTH 12
#define CNPJ_LENGTH 20

typedef struct {
char code[CODE_LENGTH];
char real_cnpj[CNPJ_LENGTH];
char received_cnpj[CNPJ_LENGTH];
char real_weight;
char received_weight;
int test_cnpj; // CNPJ correto = 1, CNPJ errado = 0 - variavel para ordenar por prioridade
int test_weight; // Peso =< limite = 1, ultrapassou mais de 10% do limite (real_weight) = 0
int id; // Ordem de cadastro
} container;


int find_code(container* list, int size, const char* code_find) {
    for (int i = 0; i < size; i++) {
        if (strcmp(list[i].code, code_find) == 0) {
            return i;
        }
    }
    return -1;
}



container* read_file(const char* filename, int* listsize_p) {


}




int main (int argc, char *argv[]) {



    return 0;
}