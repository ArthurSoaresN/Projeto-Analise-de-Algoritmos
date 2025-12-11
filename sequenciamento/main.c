#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_NOME 10
#define MAX_DNA 1001

typedef struct {
    char nome[MAX_NOME];
    double probabilidade;
    int id;
} Doenca;

Doenca* read_file(const char* filename, int* qtd_doencas_out) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    int K; 
    if (fscanf(file, "%d\n", &K) != 1) return NULL;

    char* sequenciamento = (char*)malloc(MAX_DNA * sizeof(char));
    if (!sequenciamento) return NULL;
    fscanf(file, "%s\n", sequenciamento);

    int numero_doencas = 0;
    fscanf(file, "%d\n", &numero_doencas);
    *qtd_doencas_out = numero_doencas;

    Doenca* lista_doencas = (Doenca*)malloc(numero_doencas * sizeof(Doenca));

    for (int i = 0; i < numero_doencas; i++) {
        int numero_genes;
        fscanf(file, "%s %d", lista_doencas[i].nome, &numero_genes);
        lista_doencas[i].id = i; 
        int genes_detectados = 0; 

        for (int g = 0; g < numero_genes; g++) {
            char gene_atual[1005]; 
            fscanf(file, "%s", gene_atual);

            int len_gene = strlen(gene_atual);
            int total_subcadeias = len_gene - K + 1;
            int subcadeias_encontradas = 0;

            if (total_subcadeias > 0) {
                char* sub = (char*)malloc((K + 1) * sizeof(char));
                for (int x = 0; x < total_subcadeias; x++) {
                    strncpy(sub, &gene_atual[x], K);
                    sub[K] = '\0'; 
                    if (busca_kmp(sequenciamento, sub)) {
                        subcadeias_encontradas++;
                    }
                }
                free(sub);

                double compatibilidade = ((double)subcadeias_encontradas / len_gene) * 100.0;
                if (compatibilidade >= 90.0) {
                    genes_detectados++;
                }
            }
        }

        if (numero_genes > 0)
            lista_doencas[i].probabilidade = ((double)genes_detectados / numero_genes) * 100.0;
        else
            lista_doencas[i].probabilidade = 0.0;
    }
    
    free(sequenciamento); 
    fclose(file);
    return lista_doencas;
}

void computar_tabela_lps(char* padrao, int M, int* lps) {
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < M) {
        if (padrao[i] == padrao[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

int busca_kmp(char* texto, char* padrao) {
    int M = strlen(padrao);
    int N = strlen(texto);
    if (M > N) return 0;
    int* lps = (int*)malloc(M * sizeof(int));
    if (!lps) return 0;
    computar_tabela_lps(padrao, M, lps);
    int i = 0;
    int j = 0;
    int encontrou = 0;
    while (i < N) {
        if (padrao[j] == texto[i]) {
            j++;
            i++;
        }
        if (j == M) {
            encontrou = 1;
            break;
        } else if (i < N && padrao[j] != texto[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
    free(lps);
    return encontrou;
}

int compare_doencas(const void* a, const void* b) {
    Doenca* dA = (Doenca*)a;
    Doenca* dB = (Doenca*)b;
    long probA = lround(dA->probabilidade);
    long probB = lround(dB->probabilidade);
    if (probA > probB) return -1;
    if (probA < probB) return 1;
    return dA->id - dB->id;
}

void merge(Doenca* arr, int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    Doenca* L = (Doenca*)malloc(n1 * sizeof(Doenca));
    Doenca* R = (Doenca*)malloc(n2 * sizeof(Doenca));

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = 0; 
    j = 0; 
    k = l; 
    while (i < n1 && j < n2) {
        long probA = lround(L[i].probabilidade);
        long probB = lround(R[j].probabilidade);

        if (probA > probB) {
            arr[k] = L[i];
            i++;
        } else if (probA < probB) {
            arr[k] = R[j];
            j++;
        } else {
            if (L[i].id < R[j].id) {
                arr[k] = L[i];
                i++;
            } else {
                arr[k] = R[j];
                j++;
            }
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

void mergesort_doencas(Doenca* arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergesort_doencas(arr, l, m);
        mergesort_doencas(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <input> <output>\n", argv[0]);
        return 1;
    }

    int numero_doencas = 0;
    Doenca* lista_doencas = read_file(argv[1], &numero_doencas);

    if (!lista_doencas) {
        printf("Erro ao ler arquivo.\n");
        return 1;
    }

    mergesort_doencas(lista_doencas, 0, numero_doencas - 1);

    FILE* outfile = fopen(argv[2], "w");
    if (!outfile) return 1;

    for (int i = 0; i < numero_doencas; i++) {
        fprintf(outfile, "%s->%.0f%%\n", lista_doencas[i].nome, lista_doencas[i].probabilidade);
    }

    fclose(outfile);
    free(lista_doencas);

    return 0;
}