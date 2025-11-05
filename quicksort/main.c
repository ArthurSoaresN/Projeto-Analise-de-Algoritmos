#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ACRONYM 2

typedef struct {
    char name_quick[ACRONYM];
    int performance;
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

results lomuto(vetor* lista_atual, int i, int j) {}

results hoare(vetor* lista_atual, int i, int j) {}

results lomuto_media (vetor* lista_atual) {}

results lomuto_aleatorio (vetor* lista_atual) {}

results lomuto_padrao (vetor* lista_atual) {}

results hoare_media (vetor* lista_atual) {}

results hoare_aleatorio (vetor* lista_atual) {}

results hoare_padrao (vetor* lista_atual) {}


int main (int argc, char *argv[]) {

    if (argc < 2) {
        return 1;
    }

    const char* input_filename = argv[1];
    int num_vetores = 0;
    vetor* todos_os_vetores = read_file(input_filename, &num_vetores);
    results* lista_resultados = (results*)malloc(num_vetores * sizeof(results));


    if (!todos_os_vetores || num_vetores == 0) {
        fprintf(stderr, "Erro de processamento.\n");
        return 1;
    }


    if (!lista_resultados) {
        perror("Erro de alocação da lista de resultados");
    }

    // LOOP para ordenar os vetores
    for (int i = 0; i < num_vetores; i++) {
        vetor vetor_atual = todos_os_vetores[i];

        // LP

        // HP

        // LA

        // HA

        // LM

        // HM

    }

    // Depois de pegar os resultados imprimir no output
     FILE* saida_file = fopen(argv[2], "w");

     fclose(saida_file);

     free(lista_resultados);
     free(todos_os_vetores);

}