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
double weight_diff_abs;
double weight_diff_perc;
int discrepancy;
} container;


int find_code(container* list, int size, const char* code_find) {
    for (int i = 0; i < size; i++) {
        if (strcmp(list[i].code, code_find) == 0) {
            return i;
        }
    }
    return -1;
}



container* read_file(const char* filename, int* listsize_p, int* listmid) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        *listsize_p = 0;
        return NULL;
    }

    int size_list = 0;
    if (fscanf(file, "%d\n", &size_list) != 1 || size_list <= 0) {
        fprintf(stderr, "Erro ao ler o numero de containers.\n");
        fclose(file);
        *listsize_p = 0;
        return NULL;
    }

    int final_index = size_list - 1;
    int initial_index = 0;
    *listmid = initial_index + (final_index - initial_index)/2;
    

    container* container_list = (container*)malloc(size_list * sizeof(container));
    if (!container_list) {
        perror("Erro ao alocar");
        fclose(file);
        *listsize_p = 0;
        return NULL;
    }
    *listsize_p = size_list;

    for(int i = 0; i < size_list; i++) {
        if (fscanf(file, "%11s %19s %ld\n", container_list[i].code, container_list[i].real_cnpj, &container_list[i].real_weight) != 3) {
            fprintf(stderr, "Erro de leitura");
            free(container_list);
            fclose(file);
            *listsize_p = 0;
            return NULL;
        }

        strcpy(container_list[i].received_cnpj, "");
        container_list[i].received_weight = -1;
        container_list[i].test_cnpj = 1;
        container_list[i].test_weight = 1;
        container_list[i].weight_diff_abs = 0.0;
        container_list[i].weight_diff_perc = 0.0;
        container_list[i].id = i;
        container_list[i].discrepancy = 0;
    }

    int n_situation = 0;
    if (fscanf(file, "%d\n", &n_situation) != 1 || n_situation < 0) {
        fprintf(stderr, "Erro de leitura");
        free(container_list);
        fclose(file);
        *listsize_p = 0;
        return NULL;
    };

    char temp_code[CODE_LENGTH];
    char temp_cnpj[CNPJ_LENGTH];
    long int temp_weight;

    for (int i = 0; i < n_situation; i++) {
        if (fscanf(file, "%11s %19s %ld", temp_code, temp_cnpj, &temp_weight) != 3) {
            fprintf(stderr, "Erro de leitura");
            free(container_list);
            fclose(file);
            *listsize_p = 0;
            return NULL;
        }
    }

    int index = find_code(container_list, size_list, temp_code);

    if (index != -1) {
        strcpy(container_list[index].received_cnpj, temp_cnpj);
        container_list[index].received_weight = temp_weight;

        if (strcmp(container_list[index].real_cnpj, container_list[index].received_cnpj) != 0) {
            container_list[index].test_cnpj = 0;
            container_list[index].discrepancy = 1;
        } else {
            container_list[index].test_cnpj = 1;
        }

        if (container_list[index].real_weight > 0) {
            container_list[index].weight_diff_abs = fabs((double)container_list[index].real_weight - container_list[index].received_weight);
            container_list[index].weight_diff_perc = (container_list[index].weight_diff_abs / container_list[index].real_weight) * 100.0;

            if (container_list[index].weight_diff_perc > 10.0) {
                container_list[index].test_weight = 0;
                container_list[index].discrepancy = 1;
            } else {
                container_list[index].test_weight = 1;
            }
        } else {
            if (container_list[index].real_weight != container_list[index].received_weight) {
                container_list[index].test_weight = 0;
                container_list[index].discrepancy = 1;
                container_list[index].weight_diff_abs = fabs((double)container_list[index].real_weight - container_list[index].received_weight);
            } else {
                container_list[index].test_weight = 1;
            }
        }
    } 
    
    else {
        pass();
    }

    fclose(file);
    return container_list;
}

container* mergsort (container* list, int start, int end) {}




int main (int argc, char *argv[]) {

    const char* input_filename = argv[1];
    int container_count = 0;
    container* list = NULL;
    int listmid = 0;
    list = read_file(input_filename, &container_count, &listmid);

    return 0;
}