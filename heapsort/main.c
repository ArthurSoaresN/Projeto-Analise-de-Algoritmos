#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int prioridade;
    unsigned char* dados;
    int tamanho_dados;
} Pacote;



void heapify(Pacote* vetor, uint32_t T, uint32_t i) {
    uint32_t P = i;
    uint32_t E = 2 * i + 1;
    uint32_t D = 2 * i + 2;

    if (E < T && vetor[E].prioridade > vetor[P].prioridade) P = E; 
    if (D < T && vetor[D].prioridade > vetor[P].prioridade) P = D;

    if (P != i) {
        Pacote temp = vetor[i];
        vetor[i] = vetor[P];
        vetor[P] = temp;
        
        heapify(vetor, T, P);
    }
}





int main (int argc, char *argv[]) {

    return 0;

}