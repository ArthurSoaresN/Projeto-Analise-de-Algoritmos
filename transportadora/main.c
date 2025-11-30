#include <stdio.h>
#include <string.h>
#include <stdlib.h>

 // Formato de entrada
    // Numero de veiculos -> para preencher a lista de veiculos

// Padrão:
    // PLACA PESO VOLUME

    // Numero de pacotes -> para preencher a lista de pacotes

// Padrão
    //  CODIGO VALOR PESO VOLUME

// SAIDA
    // [AAA1234]R$100.00,49KG(98%),10L(10%)->IJ777888999KL
    //  PLACA


#define SIZE_PLACA 10
#define SIZE_CODE 15

long int round_double(double x) {
    if (x >= 0.0)
        return (long int)(x + 0.5);
    else
        return (long int)(x - 0.5);
}

typedef struct {
	
char placa[SIZE_PLACA];
long peso;
long volume;

long peso_usado;
long volume_usado;
float valor_reais;

} veiculo;

typedef struct {

char codigo[SIZE_CODE];
float valor;
long peso;
long volume;
int pendente;

} pacote;

// recebe o arquivo e altera 2 listas na main
void read_file(const char* filename, veiculo** lista_carros, pacote** lista_encomendas, int* n_veiculos, int* n_pacotes) {
    FILE* file = fopen(filename, "r");

    int numero_veiculos = 0;
    int numero_pacotes = 0;
    
    fscanf(file, "%d\n", &numero_veiculos);
    *n_veiculos = numero_veiculos;

    *lista_carros =  (veiculo*)malloc(numero_veiculos * sizeof(veiculo));

    for (int i = 0; i < numero_veiculos; i++) {
        fscanf(file, "%8s %ld %ld\n", (*lista_carros)[i].placa, &(*lista_carros)[i].peso, &(*lista_carros)[i].volume);

        (*lista_carros)[i].valor_reais = 0.0;
        (*lista_carros)[i].peso_usado = 0;
        (*lista_carros)[i].volume_usado = 0;
    }

    fscanf(file, "%d\n", &numero_pacotes);
    *n_pacotes = numero_pacotes;

    *lista_encomendas = (pacote*)malloc(numero_pacotes * sizeof(pacote));

    for (int i = 0; i < numero_pacotes; i++){
        fscanf(file, "%14s %f %ld %ld\n", 
        (*lista_encomendas)[i].codigo, &(*lista_encomendas)[i].valor, &(*lista_encomendas)[i].peso, &(*lista_encomendas)[i].volume);
        (*lista_encomendas)[i].pendente = 1;
    }

    fclose(file);

}

void calcular_valor(float* resultado, float* tabela, int max_p, int max_v, int i, int p, int v) {
    *resultado = tabela[i * (max_p + 1) * (max_v + 1) + p * (max_v + 1) + v];
}

void colocar_valor(float* tabela, int max_p, int max_v, int i, int p, int v, float valor) {
    tabela[i * (max_p + 1) * (max_v + 1) + p * (max_v + 1) + v] = valor;
}


int processar_veiculo(FILE* saida, veiculo* caminhao, pacote* lista_geral, int qtd_total_pacotes) {
    
    int* map_indices = malloc(qtd_total_pacotes * sizeof(int));
    int n_pendentes = 0;
    
    for(int i=0; i < qtd_total_pacotes; i++) {
        if(lista_geral[i].pendente == 1) {
            map_indices[n_pendentes] = i;
            n_pendentes++;
        }
    }

    if (n_pendentes == 0) {
        fprintf(saida, "[%s]R$0.00,0KG(0%%),0L(0%%)->\n", caminhao->placa);
        free(map_indices);
        return 1;
    }

    int W = caminhao->peso;
    int V = caminhao->volume;

    long tamanho_tabela = (long)(n_pendentes + 1) * (W + 1) * (V + 1);
    float* tabela = (float*)calloc(tamanho_tabela, sizeof(float)); 

    if (!tabela) {
        perror("Erro: Memoria insuficiente para PD");
        free(map_indices);
        return 0;
    }

    for (int i = 1; i <= n_pendentes; i++) {
        int idx_real = map_indices[i-1]; // Índice na lista geral
        long peso_item = lista_geral[idx_real].peso;
        long vol_item = lista_geral[idx_real].volume;
        float val_item = lista_geral[idx_real].valor;

        for (int w = 0; w <= W; w++) {
            for (int v = 0; v <= V; v++) {
                
                float nao_levar = 0.0;
                calcular_valor(&nao_levar,tabela, W, V, i-1, w, v);
                
                float levar = 0.0;
                if (peso_item <= w && vol_item <= v) {
                    float result = 0.0;
                    calcular_valor(&result,tabela, W, V, i-1, w - peso_item, v - vol_item);
                    levar = val_item + result;
                }

                // Escolhe o melhor
                if (levar > nao_levar) {
                    colocar_valor(tabela, W, V, i, w, v, levar);
                } else {
                    colocar_valor(tabela, W, V, i, w, v, nao_levar);
                }
            }
        }
    }

    int* escolhidos = malloc(n_pendentes * sizeof(int));
    int qtd_escolhidos = 0;
    
    int w_atual = W;
    int v_atual = V;
    float valor_final = 0.0;
    calcular_valor(&valor_final, tabela, W, V, n_pendentes, W, V);
    
    caminhao->valor_reais = valor_final;
    caminhao->peso_usado = 0;
    caminhao->volume_usado = 0;

    for (int i = n_pendentes; i > 0; i--) {
        float val_atual = 0.0;
        calcular_valor(&val_atual, tabela, W, V, i, w_atual, v_atual);
        float val_cima = 0.0;
        calcular_valor(&val_cima, tabela, W, V, i-1, w_atual, v_atual);

        if (val_atual > val_cima + 0.001) {
            int idx_real = map_indices[i-1];
            
            lista_geral[idx_real].pendente = 0;
            
            escolhidos[qtd_escolhidos++] = idx_real;

            caminhao->peso_usado += lista_geral[idx_real].peso;
            caminhao->volume_usado += lista_geral[idx_real].volume;

            w_atual -= lista_geral[idx_real].peso;
            v_atual -= lista_geral[idx_real].volume;
        }
    }

    double pct_peso = (caminhao->peso > 0) ? (caminhao->peso_usado * 100.0) / caminhao->peso : 0;
    double pct_vol = (caminhao->volume > 0) ? (caminhao->volume_usado * 100.0) / caminhao->volume : 0;
    
    fprintf(saida, "[%s]R$%.2f,%ldKG(%.0f%%),%ldL(%.0f%%)->", 
        caminhao->placa,
        caminhao->valor_reais,
        caminhao->peso_usado,
        (double)round_double(pct_peso),
        caminhao->volume_usado,
        (double)round_double(pct_vol) 
    );

    for (int i = qtd_escolhidos - 1; i >= 0; i--) {
        fprintf(saida, "%s", lista_geral[escolhidos[i]].codigo);
        if (i > 0) fprintf(saida, ",");
    }
    fprintf(saida, "\n");

    free(tabela);
    free(map_indices);
    free(escolhidos);

    return 1;
}

int main (int argc, char *argv[]) {
    if(argc < 3) return 1;

    veiculo* carros_lista = NULL;
    int qtd_carros = 0;

    pacote* pacotes_lista = NULL;
    int qtd_pacotes = 0;

    read_file(argv[1], &carros_lista, &pacotes_lista, &qtd_carros, &qtd_pacotes);

    FILE* saida = fopen(argv[2], "w");
    if (!saida) {
        perror("Erro ao criar arquivo de saida");
        return 1;
    }

    for(int j = 0; j < qtd_carros; j++) {
        processar_veiculo(saida, &carros_lista[j], pacotes_lista, qtd_pacotes);
    }

    float valor_pendente = 0;
    long peso_pendente = 0;
    long vol_pendente = 0;
    int tem_pendente = 0;

    for(int i=0; i < qtd_pacotes; i++) {
        if(pacotes_lista[i].pendente == 1) {
            valor_pendente += pacotes_lista[i].valor;
            peso_pendente += pacotes_lista[i].peso;
            vol_pendente += pacotes_lista[i].volume;
            tem_pendente = 1;
        }
    }

    if (tem_pendente) {
        fprintf(saida, "PENDENTE:R$%.2f,%ldKG,%ldL->", valor_pendente, peso_pendente, vol_pendente);
        int primeiro = 1;
        for(int i=0; i < qtd_pacotes; i++) {
            if(pacotes_lista[i].pendente == 1) {
                 if(!primeiro) fprintf(saida, ",");
                 fprintf(saida, "%s", pacotes_lista[i].codigo);
                 primeiro = 0;
            }
        }
        fprintf(saida, "\n");
    } else {
        fprintf(saida, "PENDENTE:R$0.00,0KG,0L->\n");
    }

    free(carros_lista);
    free(pacotes_lista);

    return 0;
}