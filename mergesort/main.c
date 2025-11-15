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
long int real_weight;
long int received_weight;
int test_cnpj; // CNPJ correto = 1, CNPJ errado = 0 - variavel para ordenar por prioridade
int test_weight; // Peso =< limite = 1, ultrapassou mais de 10% do limite (real_weight) = 0
int id; // Ordem de cadastro
double weight_diff_abs;
double weight_pct_received;
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

container* read_file(const char* filename, int* listsize_p) {
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
            fprintf(stderr, "Erro de leitura (gabarito)\n");
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
        container_list[i].weight_pct_received = 0.0;
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
        if (fscanf(file, "%11s %19s %ld\n", temp_code, temp_cnpj, &temp_weight) != 3) {
            fprintf(stderr, "Erro de leitura\n");
            free(container_list);
            fclose(file);
            *listsize_p = 0;
            return NULL;
        }

        int index = find_code(container_list, size_list, temp_code);

        if (index != -1) {
            strcpy(container_list[index].received_cnpj, temp_cnpj);
            container_list[index].received_weight = temp_weight;

            /* CNPJ check */
            if (strcmp(container_list[index].real_cnpj, container_list[index].received_cnpj) != 0) {
                container_list[index].test_cnpj = 0;
                container_list[index].discrepancy = 1;
            } else {
                container_list[index].test_cnpj = 1;
            }

            if (container_list[index].real_weight > 0) {
                double diff = (double)container_list[index].real_weight - (double)container_list[index].received_weight;
                container_list[index].weight_diff_abs = fabs(diff);

                container_list[index].weight_pct_received = ((double)container_list[index].received_weight / (double)container_list[index].real_weight) * 100.0;

                double diff_perc = (container_list[index].weight_diff_abs / (double)container_list[index].real_weight) * 100.0;
                if (diff_perc > 10.0) {
                    container_list[index].test_weight = 0;
                    container_list[index].discrepancy = 1;
                } else {
                    container_list[index].test_weight = 1;
                }
            } else {
                if (container_list[index].real_weight != container_list[index].received_weight) {
                    container_list[index].test_weight = 0;
                    container_list[index].discrepancy = 1;
                    double diff_else = (double)container_list[index].real_weight - (double)container_list[index].received_weight;
                    container_list[index].weight_diff_abs = fabs(diff_else);
                    container_list[index].weight_pct_received = 0.0; /* undefined, keep 0 */
                } else {
                    container_list[index].test_weight = 1;
                    container_list[index].weight_pct_received = 100.0;
                    container_list[index].weight_diff_abs = 0.0;
                }
            }
        } 
        else {}
    }

    fclose(file);
    return container_list;
}

container* filter_list (container* list, int size_list, int* discrepancy_count) {

    int counting_discrepancy = 0;
    for (int i = 0; i < size_list; i++) {
        if (list[i].discrepancy == 1) {
            counting_discrepancy = counting_discrepancy + 1;
        }
    }

    if (counting_discrepancy == 0) {
        *discrepancy_count = 0;
        return NULL;
    }

    *discrepancy_count = counting_discrepancy;

    container* filtered = (container*)malloc(counting_discrepancy * sizeof(container));
    if (!filtered) {
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < size_list; i++) {
        if (list[i].discrepancy == 1) {
            filtered[j++] = list[i];
        }
    }

    return filtered;

}

int compare_containers (const container* a, const container* b){

    if (a->test_cnpj == 0 && b->test_cnpj == 1) {
        return -1;
    }

    if (a->test_cnpj == 1 && b->test_cnpj == 0) {
        return 1;  // 'b' (CNPJ errado) vem antes de 'a' (CNPJ certo)
    }

    if (a->test_cnpj == 0 && b->test_cnpj == 0) {
        return a->id - b->id; // Desempate pela ordem de cadastro
    }

    if (a->test_weight == 0 && b->test_weight == 1) {
        return -1; // 'a' (Peso errado) vem antes de 'b' (Peso certo)
    }

    if (a->test_weight == 1 && b->test_weight == 0) {
        return 1;  // 'b' (Peso errado) vem antes de 'a' (Peso certo)
    }

    if (a->test_weight == 0 && b->test_weight == 0) {
        if (a->weight_pct_received > b->weight_pct_received) return -1;
        if (a->weight_pct_received < b->weight_pct_received) return 1;
        return a->id - b->id;
    }

    return a->id - b->id;

}

void intercalar(container* S, int i, int m, int j) {
    int i1 = i;     // Início do primeiro sub-vetor
    int i2 = m + 1; // Início do segundo sub-vetor
    int k = 0;      // Índice do vetor auxiliar
    int tam_aux = (j - i + 1);

    container* aux = (container*)malloc(tam_aux * sizeof(container));
    if (!aux) {
        perror("Erro ao alocar memoria");
        exit(1);
    }

    while (i1 <= m && i2 <= j) {
        if (compare_containers(&S[i1], &S[i2]) <= 0) {
            aux[k++] = S[i1++];
        } else {
            aux[k++] = S[i2++];
        }
    }

    while (i1 <= m) {
        aux[k++] = S[i1++];
    }
    
    while (i2 <= j) {
        aux[k++] = S[i2++];
    }

    for (k = 0; k < tam_aux; k++) {
        S[i + k] = aux[k];
    }
    
    free(aux);
}

void mergesort (container* list, int start, int end) {
    if (start < end) {
    int m = start + (end - start)/2;
    
    mergesort(list, start, m);
    mergesort(list, m+1, end);

    intercalar(list, start, m, end);
    }
}




int main (int argc, char *argv[]) {

    if (argc < 2) {
        return 1;
    }

    const char* input_filename = argv[1];
    container* list = NULL;
    container* filtered_list = NULL;
    int container_count = 0;
    int discrepancy_count = 0;

    list = read_file(input_filename, &container_count);

    if (list == NULL || container_count == 0) {
        return 1;
    }

    filtered_list = filter_list(list, container_count, &discrepancy_count);
    free(list);

    if (filtered_list == NULL) {
        return 1;
    }

    mergesort(filtered_list, 0, discrepancy_count - 1);

    FILE* saida_file = fopen(argv[2], "w");

    if (saida_file == NULL) {
        perror("Erro ao criar o arquivo de saida");
        free(filtered_list);
        return 1; // Retorna erro
    }

    for (int i = 0; i < discrepancy_count; i++) {
        container c = filtered_list[i];

        if (c.test_cnpj == 0) {
            fprintf(saida_file, "%s:%s<->%s\n", c.code, c.real_cnpj, c.received_cnpj);
        } else if (c.test_weight == 0) {
            fprintf(saida_file, "%s:%ldkg(%.0f%%)\n",
                    c.code,
                    c.received_weight,
                    round(c.weight_pct_received));
        }
    }

    fclose(saida_file);
    free(filtered_list);

    return 0;
}