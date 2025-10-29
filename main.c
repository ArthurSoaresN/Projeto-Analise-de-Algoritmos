#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {

char code;
char real_cnpj;
char received_cnpj;
char real_weight;
char received_weight;
int test_cnpj; // CNPJ correto = 1, CNPJ errado = 0 - variavel para ordenar por prioridade
int test_weight; // Peso =< limite = 1, ultrapassou mais de 10% do limite (real_weight) = 0
int id; // Ordem de cadastro

} container;

void read_file(const char* filename) {

    int size_list = NULL;

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return;
    }

    if (size_list > 1) {
        container container_list = [size_list];
    }

    // ler o arquivo aqui...

    fclose(file);
}




int main (int argc, char *argv[]) {



    return 0;
}