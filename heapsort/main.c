#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int prioridade;
    unsigned char* dados;
    int tamanho_dados;

} Pacote;

void trocar(Pacote* a, Pacote* b) {
    Pacote temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(Pacote* V, int T, int i) {
    int P = i;
    int E = 2 * i + 1;
    int D = 2 * i + 2;

    if (E < T && V[E].prioridade > V[P].prioridade) {
        P = E;
    }
    if (D < T && V[D].prioridade > V[P].prioridade) {
        P = D;
    }

    if (P != i) {
        trocar(&V[i], &V[P]);
        heapify(V, T, P);
    }
}

void construir_heap(Pacote* V, int n) {
    for (int i = (n - 2) / 2; i >= 0; i--) {
        heapify(V, n, i);
    }
}

void heapsort(Pacote* V, int n) {
    construir_heap(V, n);
    for (int i = n - 1; i > 0; i--) {
        trocar(&V[0], &V[i]);
        heapify(V, i, 0);
    }
}

void write_output(FILE* file, Pacote* buffer, int num_pacotes) {
    for (int i = num_pacotes - 1; i >= 0; i--) {
        fprintf(file, "|");
        for (int j = 0; j < buffer[i].tamanho_dados; j++) {
            fprintf(file, "%02X", buffer[i].dados[j]);
            if (j < buffer[i].tamanho_dados - 1) {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "|\n");
}

void clear_buffer(Pacote* buffer, int num_pacotes) {
    for (int i = 0; i < num_pacotes; i++) {
        free(buffer[i].dados);
        buffer[i].dados = NULL;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }

    FILE* input_file = fopen(argv[1], "r");
    if (!input_file) {
        perror("Erro ao abrir input");
        return 1;
    }

    FILE* output_file = fopen(argv[2], "w");
    if (!output_file) {
        perror("Erro ao criar output");
        fclose(input_file);
        return 1;
    }

    int n_pacotes_total = 0;
    int limite_bytes = 0;
    if (fscanf(input_file, "%d %d\n", &n_pacotes_total, &limite_bytes) != 2) {
        fprintf(stderr, "Erro ao ler Limite de Bytes).\n");
        fclose(input_file);
        fclose(output_file);
        return 1;
    }

    Pacote* buffer_pacotes = (Pacote*)malloc(n_pacotes_total * sizeof(Pacote));
    if (!buffer_pacotes) {
        perror("Erro ao alocar buffer principal");
        fclose(input_file);
        fclose(output_file);
        return 1;
    }

    int pacotes_no_buffer = 0;
    int bytes_no_buffer = 0;

    for (int i = 0; i < n_pacotes_total; i++) {
        
        int prioridade_atual;
        int tamanho_atual;

        if (fscanf(input_file, "%d %d", &prioridade_atual, &tamanho_atual) != 2) {
            fprintf(stderr, "Erro ao ler linha %d do pacote.\n", i + 2);
            break;
        }

        if (bytes_no_buffer + tamanho_atual > limite_bytes && pacotes_no_buffer > 0) {
            
            heapsort(buffer_pacotes, pacotes_no_buffer);
            
            write_output(output_file, buffer_pacotes, pacotes_no_buffer);
            
            clear_buffer(buffer_pacotes, pacotes_no_buffer);
            
            pacotes_no_buffer = 0;
            bytes_no_buffer = 0;
        }

        buffer_pacotes[pacotes_no_buffer].prioridade = prioridade_atual;
        buffer_pacotes[pacotes_no_buffer].tamanho_dados = tamanho_atual;
        
        buffer_pacotes[pacotes_no_buffer].dados = (unsigned char*)malloc(tamanho_atual * sizeof(unsigned char));
        if (!buffer_pacotes[pacotes_no_buffer].dados) {
            perror("Erro ao alocar memoria para os bytes do pacote");
            break;
        }

        for (int j = 0; j < tamanho_atual; j++) {
            unsigned int byte_lido;
            if (fscanf(input_file, "%x", &byte_lido) != 1) { 
                fprintf(stderr, "Erro ao ler os bytes do pacote.\n");
                i = n_pacotes_total; 
                break;
            }
            buffer_pacotes[pacotes_no_buffer].dados[j] = (unsigned char)byte_lido;
        }
        
        bytes_no_buffer += tamanho_atual;
        pacotes_no_buffer++;

        fscanf(input_file, "\n");
    }

    if (pacotes_no_buffer > 0) {
        heapsort(buffer_pacotes, pacotes_no_buffer);
        write_output(output_file, buffer_pacotes, pacotes_no_buffer);
        clear_buffer(buffer_pacotes, pacotes_no_buffer);
    }

    free(buffer_pacotes);
    fclose(input_file);
    fclose(output_file);

    return 0;
}