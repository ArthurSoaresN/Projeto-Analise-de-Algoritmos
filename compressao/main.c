#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_SIZE 10005
#define MAX_HEX_OUTPUT (MAX_SIZE * 3)

typedef struct {
    int size_bytes;
    char *hex_string;
    float taxa_compressao;
} CompressaoResult;

CompressaoResult executar_RLE(unsigned char *dados, int n) {
    CompressaoResult res;
    res.hex_string = (char *)calloc(n * 4 + 1, sizeof(char)); 

    if (n == 0) {
        res.size_bytes = 0;
        res.taxa_compressao = 0.0;
        return res;
    }

    int cursor = 0;
    int offset_str = 0; 
    int total_bytes = 0;

    while (cursor < n) {
        unsigned char valor = dados[cursor];
        int repeticoes = 1;
        int proximo = cursor + 1;

        while (proximo < n && dados[proximo] == valor) {
            if (repeticoes >= 255) break;
            repeticoes++;
            proximo++;
        }

        offset_str += sprintf(res.hex_string + offset_str, "%02X%02X", repeticoes, valor);
        total_bytes += 2;
        cursor = proximo; 
    }

    res.size_bytes = total_bytes;
    res.taxa_compressao = ((float)total_bytes / n) * 100.0;
    return res;
}

typedef struct MinHeapNode {
    unsigned char dado;
    unsigned int freq;
    struct MinHeapNode *esq, *dir;
} MinHeapNode;

typedef struct MinHeap {
    unsigned int tamanho;
    unsigned int capacidade;
    MinHeapNode **array;
} MinHeap;

MinHeapNode* novoNo(unsigned char dado, unsigned freq) {
    MinHeapNode* temp = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    temp->esq = temp->dir = NULL;
    temp->dado = dado;
    temp->freq = freq;
    return temp;
}

MinHeap* criarMinHeap(unsigned capacidade) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->tamanho = 0;
    minHeap->capacidade = capacidade;
    minHeap->array = (MinHeapNode**)malloc(minHeap->capacidade * sizeof(MinHeapNode*));
    return minHeap;
}

void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int menor = idx;
    int esq = 2 * idx + 1;
    int dir = 2 * idx + 2;

    if (esq < minHeap->tamanho && minHeap->array[esq]->freq < minHeap->array[menor]->freq)
        menor = esq;

    if (dir < minHeap->tamanho && minHeap->array[dir]->freq < minHeap->array[menor]->freq)
        menor = dir;

    if (menor != idx) {
        swapMinHeapNode(&minHeap->array[menor], &minHeap->array[idx]);
        minHeapify(minHeap, menor);
    }
}

MinHeapNode* extractMin(MinHeap* minHeap) {
    MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->tamanho - 1];
    --minHeap->tamanho;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap* minHeap, MinHeapNode* minHeapNode) {
    ++minHeap->tamanho;
    int i = minHeap->tamanho - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

int isSizeOne(MinHeap* minHeap) {
    return (minHeap->tamanho == 1);
}

MinHeapNode* construirArvoreHuffman(unsigned char dados[], int freq[], int tamanho) {
    MinHeapNode *esq, *dir, *top;
    MinHeap* minHeap = criarMinHeap(tamanho);

    for (int i = 0; i < tamanho; ++i)
        insertMinHeap(minHeap, novoNo(dados[i], freq[i]));

    while (!isSizeOne(minHeap)) {
        esq = extractMin(minHeap);
        dir = extractMin(minHeap);
        top = novoNo('$', esq->freq + dir->freq);
        top->esq = esq;
        top->dir = dir;
        insertMinHeap(minHeap, top);
    }
    MinHeapNode* raiz = extractMin(minHeap);
    free(minHeap->array);
    free(minHeap);
    return raiz;
}

int gerarCodigos(MinHeapNode* raiz, int arr[], int top, char tabelaCodigos[256][256]) {
    if (raiz->esq) {
        arr[top] = 0;
        gerarCodigos(raiz->esq, arr, top + 1, tabelaCodigos);
    }
    if (raiz->dir) {
        arr[top] = 1;
        gerarCodigos(raiz->dir, arr, top + 1, tabelaCodigos);
    }
    if (!(raiz->esq) && !(raiz->dir)) {
        if (top == 0) {
            tabelaCodigos[raiz->dado][0] = '0';
            tabelaCodigos[raiz->dado][1] = '\0';
        } else {
            int i = 0;
            while (i < top) {
                tabelaCodigos[raiz->dado][i] = arr[i] + '0';
                i++;
            }
            tabelaCodigos[raiz->dado][top] = '\0';
        }
    }
    return 0;
}

int liberarArvore(MinHeapNode* raiz) {
    if (raiz == NULL) return 0;
    liberarArvore(raiz->esq);
    liberarArvore(raiz->dir);
    free(raiz);
    return 0;
}

void adicionar_bit(unsigned char *buffer, int *bit_count, char bit, char *destino, int *write_idx, int *total_bytes) {
    if (bit == '1') {
        *buffer = *buffer | (1 << (7 - *bit_count));
    }
    (*bit_count)++;
    if (*bit_count == 8) {
        *write_idx += sprintf(destino + *write_idx, "%02X", *buffer);
        (*total_bytes)++;
        *buffer = 0;
        *bit_count = 0;
    }
}

CompressaoResult executar_Huffman(unsigned char *dados, int n) {
    CompressaoResult res;
    res.hex_string = (char *)malloc(n * 8 + 100); 

    if (n == 0) {
        res.size_bytes = 0;
        res.hex_string[0] = '\0';
        res.taxa_compressao = 0.0;
        return res;
    }

    int freq[256] = {0};
    int i = 0;          
    while (i < n) {    
        freq[dados[i]]++;
        i++;          
    }

    unsigned char arr_chars[256];
    int arr_freq[256];
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            arr_chars[size] = (unsigned char)i;
            arr_freq[size] = freq[i];
            size++;
        }
    }

    MinHeapNode* raiz = construirArvoreHuffman(arr_chars, arr_freq, size);

    char tabelaCodigos[256][256];
    int arr_aux[256];
    memset(tabelaCodigos, 0, sizeof(tabelaCodigos));
    gerarCodigos(raiz, arr_aux, 0, tabelaCodigos);

    unsigned char buffer = 0;
    int bit_count = 0;
    int write_idx = 0;
    int total_bytes = 0;

    for (int i = 0; i < n; i++) {
        unsigned char byte_atual = dados[i];
        char *codigo = tabelaCodigos[byte_atual];
        int k = 0;
        while (codigo[k] != '\0') {
            adicionar_bit(&buffer, &bit_count, codigo[k], res.hex_string, &write_idx, &total_bytes);
            k++;
        }
    }

    if (bit_count > 0) {
        write_idx += sprintf(&res.hex_string[write_idx], "%02X", buffer);
        total_bytes++;
    }

    res.size_bytes = total_bytes;
    res.taxa_compressao = ((float)res.size_bytes / n) * 100.0;

    liberarArvore(raiz);
    return res;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    FILE *fout = fopen(argv[2], "w");

    if (!fin || !fout) {
        return 1;
    }

    int num_sequencias;
    if (fscanf(fin, "%d", &num_sequencias) != 1) return 1;

    unsigned char *dados = (unsigned char *)malloc(MAX_SIZE * sizeof(unsigned char));

    for (int seq = 0; seq < num_sequencias; seq++) {
        int tamanho_seq;
        fscanf(fin, "%d", &tamanho_seq);
        for (int i = 0; i < tamanho_seq; i++) {
            unsigned int temp_val;
            fscanf(fin, "%x", &temp_val);
            dados[i] = (unsigned char)temp_val;
        }

        CompressaoResult rle = executar_RLE(dados, tamanho_seq);
        CompressaoResult huf = executar_Huffman(dados, tamanho_seq);

        if (huf.size_bytes == rle.size_bytes) {
            fprintf(fout, "%d->HUF(%.2f%%)=%s\n", seq, huf.taxa_compressao, huf.hex_string);
            fprintf(fout, "%d->RLE(%.2f%%)=%s\n", seq, rle.taxa_compressao, rle.hex_string);
        }
        else if (huf.size_bytes < rle.size_bytes) {
            fprintf(fout, "%d->HUF(%.2f%%)=%s\n", seq, huf.taxa_compressao, huf.hex_string);
        }
        else {
            fprintf(fout, "%d->RLE(%.2f%%)=%s\n", seq, rle.taxa_compressao, rle.hex_string);
        }

        free(rle.hex_string);
        free(huf.hex_string);
    }

    free(dados);
    fclose(fin);
    fclose(fout);

    return 0;
}
