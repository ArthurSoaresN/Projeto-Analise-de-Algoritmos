#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ACRONYM 3

typedef struct {
    char name_quick[ACRONYM];
    long performance;
} results;

typedef struct {
    int* elementos;
    int tamanho; 
} vetor;

vetor* read_file(const char* filename, int* num_vetores_ptr) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro");
        *num_vetores_ptr = 0;
        return NULL;
    }

    int N = 0;

    if (fscanf(file, "%d\n", &N) != 1 || N <= 0) {
        fprintf(stderr, "Erro\n");
        fclose(file);
        *num_vetores_ptr = 0;
        return NULL;
    }

    vetor* lista_de_vetores = (vetor*)malloc(N * sizeof(vetor));

    if (!lista_de_vetores) {
        perror("Erro");
        fclose(file);
        *num_vetores_ptr = 0;
        return NULL;
    }

    *num_vetores_ptr = N;

    for (int i = 0; i < N; i++) {
    
        int M_i = 0;
        
        if (fscanf(file, "%d\n", &M_i) != 1 || M_i < 0) {
            fprintf(stderr, "Erro ao ler o tamanho do vetor %d.\n", i + 1);
    
            for (int k = 0; k < i; k++) { // Libera vetores internos já alocados
                free(lista_de_vetores[k].elementos);
            }

            free(lista_de_vetores); // Libera o array principal
            fclose(file);
            *num_vetores_ptr = 0;
            return NULL;
        }

        lista_de_vetores[i].tamanho = M_i;

        if (M_i == 0) {
             lista_de_vetores[i].elementos = NULL;
             continue;
        }

        lista_de_vetores[i].elementos = (int*)malloc(M_i * sizeof(int));

        if (!lista_de_vetores[i].elementos) {
            perror("Erro ao alocar memoria para elementos do vetor");
            for (int k = 0; k < i; k++) { 
                free(lista_de_vetores[k].elementos);
            }
            free(lista_de_vetores);
            fclose(file);
            *num_vetores_ptr = 0;
            return NULL;
        }


        for (int j = 0; j < M_i; j++) {
            if (fscanf(file, "%d", &lista_de_vetores[i].elementos[j]) != 1) {
                fprintf(stderr, "Erro ao ler elemento %d do vetor %d.\n", j + 1, i + 1);
                free(lista_de_vetores[i].elementos);
                for (int k = 0; k < i; k++) { 
                    free(lista_de_vetores[k].elementos);
                }
                free(lista_de_vetores);
                fclose(file);
                *num_vetores_ptr = 0;
                return NULL;
            }
        }
        fscanf(file, "\n"); 
    }

    fclose(file);
    return lista_de_vetores;
}

void funcao_trocar(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void sort_results(results r[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (r[j].performance > r[j+1].performance) {
                results temp = r[j];
                r[j] = r[j+1];
                r[j+1] = temp;
            }
        }
    }
}

int get_median_index(int* V, int i, int j) {
    int n_sub = j - i + 1;
    int idx1 = i + (n_sub / 4);
    int idx2 = i + (n_sub / 2);
    int idx3 = i + (3 * n_sub / 4);

    int val1 = V[idx1];
    int val2 = V[idx2];
    int val3 = V[idx3];

    if ((val1 <= val2 && val2 <= val3) || (val3 <= val2 && val2 <= val1)) return idx2;
    if ((val2 <= val1 && val1 <= val3) || (val3 <= val1 && val1 <= val2)) return idx1;
    return idx3;
}

int lomuto(int* V, int start, int end, long* trocas) {
    int pivo = V[end];
    int i = start - 1;

    for (int j = start; j < end; j++) {
        if (V[j] <= pivo) {
            i++;
            funcao_trocar(&V[i], &V[j]);
            (*trocas)++;
        }
    }
    funcao_trocar(&V[i + 1], &V[end]);
    (*trocas)++;
    return i + 1;
}

int hoare(int* V, int i, int j, long* trocas) {
    int P = V[i];
    int x = i - 1;
    int y = j + 1;

    while (1) {
        while (V[--y] > P);
        while (V[++x] < P);
        if (x < y) {
            funcao_trocar(&V[x], &V[y]);
            (*trocas)++;
        } else {
            return y;
        }
    }
}

void quicksort_HP(int* V, int i, int j, long* trocas, long* chamadas) {
    (*chamadas)++;

    if (i < j) {
        // Chama o particionamento
        int p = hoare(V, i, j, trocas);

        // Recursão (atenção aos limites!)
        quicksort_HP(V, i, p, trocas, chamadas);
        quicksort_HP(V, p + 1, j, trocas, chamadas);
    }
}

void quicksort_LP(int* V, int start, int end, long* trocas, long* chamadas) {
    (*chamadas)++;
    if (start < end) {
        int p = lomuto(V, start, end, trocas);
        quicksort_LP(V, start, p - 1, trocas, chamadas);
        quicksort_LP(V, p + 1, end, trocas, chamadas);
    }
}

void quicksort_LA(int* V, int start, int end, long* trocas, long* chamadas) {
    (*chamadas)++;
    if (start < end) {
        // Fórmula do exercício: r = i + (|V[i]| % n_sub)
        int n_sub = end - start + 1;
        int r = start + (abs(V[start]) % n_sub);
        
        funcao_trocar(&V[r], &V[end]); // Lomuto espera pivô no fim
        (*trocas)++;

        int p = lomuto(V, start, end, trocas);
        quicksort_LA(V, start, p - 1, trocas, chamadas);
        quicksort_LA(V, p + 1, end, trocas, chamadas);
    }
}

void quicksort_HA(int* V, int start, int end, long* trocas, long* chamadas) {
    (*chamadas)++;
    if (start < end) {
        int n_sub = end - start + 1;
        int r = start + (abs(V[start]) % n_sub);
        
        funcao_trocar(&V[r], &V[start]); // Hoare espera pivô no início
        (*trocas)++;

        int p = partition_hoare(V, start, end, trocas);
        quicksort_HA(V, start, p, trocas, chamadas);
        quicksort_HA(V, p + 1, end, trocas, chamadas);
    }
}

void quicksort_LM(int* V, int start, int end, long* trocas, long* chamadas) {
    (*chamadas)++;
    if (start < end) {
        int med_idx = get_median_index(V, start, end);
        
        funcao_trocar(&V[med_idx], &V[end]); 
        (*trocas)++;

        int p = lomuto(V, start, end, trocas);
        quicksort_LM(V, start, p - 1, trocas, chamadas);
        quicksort_LM(V, p + 1, end, trocas, chamadas);
    }
}

void quicksort_HM(int* V, int start, int end, long* trocas, long* chamadas) {
    (*chamadas)++;
    if (start < end) {
        int med_idx = get_median_index(V, start, end);
        
        funcao_trocar(&V[med_idx], &V[start]);
        (*trocas)++;

        int p = partition_hoare(V, start, end, trocas);
        quicksort_HM(V, start, p, trocas, chamadas);
        quicksort_HM(V, p + 1, end, trocas, chamadas);
    }
}

int main (int argc, char *argv[]) {

    if (argc < 3) {
        return 1;
    }

    const char* input_filename = argv[1];
    int num_vetores = 0;
    vetor* lista_vetores = read_file(input_filename, &num_vetores);

    if (!lista_vetores || num_vetores == 0) {
        fprintf(stderr, "Erro de processamento ou arquivo vazio.\n");
        return 1;
    }

    FILE* saida = fopen(argv[2], "w");
    if (!saida) {
        perror("Erro ao criar arquivo de saida");
        for (int i = 0; i < num_vetores; i++) free(lista_vetores[i].elementos);
        free(lista_vetores);
        return 1;
    }

    // LOOP para ordenar os vetores
    for (int i = 0; i < num_vetores; i++) {
        vetor vetor_atual = lista_vetores[i];

        if (vetor_atual.tamanho <= 0) continue;

        results lista_resultados[6];
        long trocas, chamadas;

        // LP
        int* copia_lp = (int*)malloc(vetor_atual.tamanho * sizeof(int));
        memcpy(copia_lp, vetor_atual.elementos, vetor_atual.tamanho * sizeof(int));
        trocas = 0; chamadas = 0;
        quicksort_LP(copia_lp, 0, vetor_atual.tamanho - 1, &trocas, &chamadas);
        strcpy(lista_resultados[0].name_quick, "LP");
        lista_resultados[0].performance = trocas + chamadas;
        free(copia_lp);

        // HP
        int* copia_hp = (int*)malloc(vetor_atual.tamanho * sizeof(int));
        memcpy(copia_hp, vetor_atual.elementos, vetor_atual.tamanho * sizeof(int));
        trocas = 0; chamadas = 0;
        quicksort_HP(copia_hp, 0, vetor_atual.tamanho - 1, &trocas, &chamadas);
        strcpy(lista_resultados[1].name_quick, "HP");
        lista_resultados[1].performance = trocas + chamadas;
        free(copia_hp);

        // LA
        int* copia_la = (int*)malloc(vetor_atual.tamanho * sizeof(int));
        memcpy(copia_la, vetor_atual.elementos, vetor_atual.tamanho * sizeof(int));
        trocas = 0; chamadas = 0;
        quicksort_LA(copia_la, 0, vetor_atual.tamanho - 1, &trocas, &chamadas);
        strcpy(lista_resultados[2].name_quick, "LA");
        lista_resultados[2].performance = trocas + chamadas;
        free(copia_la);

        // HA
        int* copia_ha = (int*)malloc(vetor_atual.tamanho * sizeof(int));
        memcpy(copia_ha, vetor_atual.elementos, vetor_atual.tamanho * sizeof(int));
        trocas = 0; chamadas = 0;
        quicksort_HA(copia_ha, 0, vetor_atual.tamanho - 1, &trocas, &chamadas);
        strcpy(lista_resultados[3].name_quick, "HA");
        lista_resultados[3].performance = trocas + chamadas;
        free(copia_ha);

        // LM
        int* copia_lm = (int*)malloc(vetor_atual.tamanho * sizeof(int));
        memcpy(copia_lm, vetor_atual.elementos, vetor_atual.tamanho * sizeof(int));
        trocas = 0; chamadas = 0;
        quicksort_LM(copia_lm, 0, vetor_atual.tamanho - 1, &trocas, &chamadas);
        strcpy(lista_resultados[4].name_quick, "LM");
        lista_resultados[4].performance = trocas + chamadas;
        free(copia_lm);

        // HM
        int* copia_hm = (int*)malloc(vetor_atual.tamanho * sizeof(int));
        memcpy(copia_hm, vetor_atual.elementos, vetor_atual.tamanho * sizeof(int));
        trocas = 0; chamadas = 0;
        quicksort_HM(copia_hm, 0, vetor_atual.tamanho - 1, &trocas, &chamadas);
        strcpy(lista_resultados[5].name_quick, "HM");
        lista_resultados[5].performance = trocas + chamadas;
        free(copia_hm);

        sort_results(lista_resultados, 6);

        fprintf(saida, "[%d]:", vetor_atual.tamanho);
        for (int k = 0; k < 6; k++) {
            fprintf(saida, "%s(%ld)", lista_resultados[k].name_quick, lista_resultados[k].performance);
            if (k < 5) fprintf(saida, ",");
        }
        fprintf(saida, "\n");
    }

    fclose(saida);
    for (int i = 0; i < num_vetores; i++) {
        free(lista_vetores[i].elementos);
    }
    free(lista_vetores);

    return 0;
}