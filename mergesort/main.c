#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CODE_LENGTH 12
#define CNPJ_LENGTH 20

// --- FUNÇÕES AUXILIARES ---

long int round_double(double x) {
    if (x >= 0.0)
        return (long int)(x + 0.5);
    else
        return (long int)(x - 0.5);
}

long int abs_long_diff(long int a, long int b) {
    if (a >= b) return a - b;
    return b - a;
}

typedef struct {
    char code[CODE_LENGTH];
    char real_cnpj[CNPJ_LENGTH];
    char received_cnpj[CNPJ_LENGTH];
    long int real_weight;
    long int received_weight;
    int test_cnpj;
    int test_weight;
    int id;
    double weight_diff_abs;
    double weight_pct_received;
    int discrepancy;
} container;

// Função de comparação para o qsort e bsearch (ordena por CÓDIGO)
int compare_by_code(const void *a, const void *b) {
    container *cA = (container *)a;
    container *cB = (container *)b;
    return strcmp(cA->code, cB->code);
}

// --- LEITURA DO ARQUIVO (OTIMIZADA COM BUSCA BINÁRIA) ---

container* read_file(const char* filename, int* listsize_p) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        *listsize_p = 0;
        return NULL;
    }

    int size_list = 0;
    if (fscanf(file, "%d\n", &size_list) != 1 || size_list <= 0) {
        fprintf(stderr, "Erro ao ler quantidade\n");
        fclose(file);
        *listsize_p = 0;
        return NULL;
    }

    container* container_list = malloc(size_list * sizeof(container));
    if (!container_list) {
        perror("Erro malloc");
        fclose(file);
        *listsize_p = 0;
        return NULL;
    }
    *listsize_p = size_list;

    // 1. Ler o Gabarito
    for (int i = 0; i < size_list; i++) {
        if (fscanf(file, "%11s %19s %ld\n",
                container_list[i].code,
                container_list[i].real_cnpj,
                &container_list[i].real_weight) != 3) 
        {
            fprintf(stderr, "Erro leitura gabarito na linha %d\n", i+1);
            free(container_list); 
            fclose(file); 
            return NULL;
        }
        
        // Inicializações
        container_list[i].received_cnpj[0] = '\0';
        container_list[i].received_weight = -1;
        container_list[i].test_cnpj = 1;
        container_list[i].test_weight = 1;
        container_list[i].weight_diff_abs = 0.0;
        container_list[i].weight_pct_received = 0.0;
        container_list[i].id = i; // IMPORTANTE: Salva a ordem original antes de ordenar!
        container_list[i].discrepancy = 0;
    }

    // 2. Ordenar o gabarito por código para permitir Busca Binária (O(N log N))
    qsort(container_list, size_list, sizeof(container), compare_by_code);

    // 3. Ler a Situação
    int n_situation = 0;
    if (fscanf(file, "%d\n", &n_situation) != 1) {
         n_situation = 0;
    }

    // Variáveis temporárias para leitura da situação
    char temp_code[CODE_LENGTH];
    char temp_cnpj[CNPJ_LENGTH];
    long int temp_weight;
    
    // Chave para o bsearch
    container key; 

    for (int i = 0; i < n_situation; i++) {
        if (fscanf(file, "%11s %19s %ld\n",
                   temp_code, 
                   temp_cnpj, 
                   &temp_weight) != 3) 
        {
            fprintf(stderr, "Erro leitura situação na linha %d\n", i+1);
             free(container_list); 
             fclose(file); 
             return NULL;
        }

        // Configura a chave de busca
        strcpy(key.code, temp_code);

        // Busca Binária (Instantânea: O(log N))
        container *found = (container*) bsearch(&key, container_list, size_list, sizeof(container), compare_by_code);

        if (found != NULL) {
            // Encontrou! Atualiza os dados no ponteiro retornado (dentro da lista ordenada)
            strcpy(found->received_cnpj, temp_cnpj);
            found->received_weight = temp_weight;

            // Lógica de comparação
            if (strcmp(found->real_cnpj, found->received_cnpj) != 0) {
                found->test_cnpj = 0;
                found->discrepancy = 1;
            } else {
                found->test_cnpj = 1;
            }

            if (found->real_weight > 0) {
                found->weight_diff_abs = (double)abs_long_diff(found->real_weight, found->received_weight);
                double diff_perc = (found->weight_diff_abs / (double)found->real_weight) * 100.0;
                found->weight_pct_received = diff_perc;

                long int rounded_perc = round_double(diff_perc);
                // Regra: erro se arredondado > 10%
                if (rounded_perc > 10) {
                    found->test_weight = 0;
                    found->discrepancy = 1;
                } else {
                    found->test_weight = 1;
                }
            } else {
                // Caso peso 0 no gabarito
                if (found->received_weight != 0) {
                    found->test_weight = 0;
                    found->discrepancy = 1;
                    found->weight_diff_abs = (double)abs_long_diff(found->real_weight, found->received_weight);
                    found->weight_pct_received = 0.0; 
                } else {
                    found->test_weight = 1;
                    found->weight_diff_abs = 0.0;
                    found->weight_pct_received = 0.0;
                }
            }
            
            // Se tem qualquer erro, marca discrepância
            if (found->test_cnpj == 0 || found->test_weight == 0) {
                found->discrepancy = 1;
            }
        }
    }

    fclose(file);
    return container_list;
}

// --- FILTRAGEM ---

container* filter_list(container* list, int size_list, int* discrepancy_count) {
    int count = 0;
    for (int i = 0; i < size_list; i++) {
        if (list[i].discrepancy == 1) count++;
    }

    *discrepancy_count = count;
    if (count == 0) return NULL;

    container* filtered = malloc(count * sizeof(container));
    if (!filtered) return NULL;

    int j = 0;
    for (int i = 0; i < size_list; i++) {
        if (list[i].discrepancy) filtered[j++] = list[i];
    }
    return filtered;
}

// --- ORDENAÇÃO (MERGESORT) ---

int compare_containers(const container* a, const container* b) {
    // 1. Prioridade: Divergência de CNPJ
    if (a->test_cnpj == 0 && b->test_cnpj == 1) return -1; // a vem antes
    if (a->test_cnpj == 1 && b->test_cnpj == 0) return 1;  // b vem antes
    
    // Se empate no CNPJ (ambos errados ou ambos certos)...
    
    // 2. Prioridade: Divergência de Peso
    if (a->test_weight == 0 && b->test_weight == 1) return -1;
    if (a->test_weight == 1 && b->test_weight == 0) return 1;
    
    // Se ambos têm peso errado (test_weight == 0)...
    if (a->test_weight == 0 && b->test_weight == 0) {
        // 3. Prioridade: Maior diferença percentual (Arredondada)
        long int ra = round_double(a->weight_pct_received);
        long int rb = round_double(b->weight_pct_received);

        if (ra > rb) return -1; // Maior vem antes
        if (ra < rb) return 1;
        
        // 4. Desempate final: Ordem de cadastro (ID)
        return a->id - b->id;
    }
    
    // Desempate geral por ID
    return a->id - b->id;
}

void intercalar(container* S, int i, int m, int j) {
    int i1 = i, i2 = m + 1, k = 0;
    int tam = j - i + 1;

    container* aux = malloc(tam * sizeof(container));
    if (!aux) exit(1);

    while (i1 <= m && i2 <= j) {
        if (compare_containers(&S[i1], &S[i2]) <= 0)
            aux[k++] = S[i1++];
        else
            aux[k++] = S[i2++];
    }

    while (i1 <= m) aux[k++] = S[i1++];
    while (i2 <= j) aux[k++] = S[i2++];

    for (k = 0; k < tam; k++) S[i + k] = aux[k];

    free(aux);
}

void mergesort(container* list, int start, int end) {
    if (start < end) {
        int m = start + (end - start) / 2; 
        mergesort(list, start, m);
        mergesort(list, m + 1, end);
        intercalar(list, start, m, end);
    }
}

// --- MAIN ---

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s entrada.txt saida.txt\n", argv[0]);
        return 1;
    }

    int container_count = 0;
    // Leitura otimizada com QSort e Binary Search
    container* list = read_file(argv[1], &container_count);
    
    if (!list) return 1; 

    int discrepancy_count = 0;
    container* filtered = filter_list(list, container_count, &discrepancy_count);
    
    free(list); // Libera a memória da lista completa

    // Se não houver discrepâncias, cria arquivo vazio e sai
    if (!filtered || discrepancy_count == 0) {
        FILE* out = fopen(argv[2], "w");
        if (out) fclose(out);
        if (filtered) free(filtered);
        return 0;
    }

    // Ordena apenas os filtrados
    mergesort(filtered, 0, discrepancy_count - 1);

    FILE* out = fopen(argv[2], "w");
    if (!out) {
        perror("Erro escrever");
        free(filtered);
        return 1;
    }

    for (int i = 0; i < discrepancy_count; i++) {
        container c = filtered[i];

        if (c.test_cnpj == 0) {
            fprintf(out, "%s:%s<->%s\n",
                    c.code, c.real_cnpj, c.received_cnpj);
        }
        else if (c.test_weight == 0) {
            long int diferenca_kg = abs_long_diff(c.received_weight, c.real_weight);
            fprintf(out, "%s:%ldkg(%.0f%%)\n",
                    c.code,
                    diferenca_kg,
                    c.weight_pct_received);
        }
    }

    fclose(out);
    free(filtered);
    return 0;
}