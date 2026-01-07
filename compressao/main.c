#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_SIZE 10005

typedef struct {
    int id;
    int tamanho;
    unsigned char *dados;
} SequenciaInput;

typedef struct {
    int size_bytes;
    char *hex_string;
    float taxa_compressao;
} CompressaoResult;

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

SequenciaInput* read_file(const char *filename, int *qtd_out);
void free_input(SequenciaInput *lista, int qtd);
CompressaoResult executar_RLE(unsigned char *dados, int n);
CompressaoResult executar_Huffman(unsigned char *dados, int n);
void adicionar_bit(unsigned char *buffer, int *bit_count, char bit, char *destino, int *write_idx, int *total_bytes);

MinHeapNode* novoNo(unsigned char dado, unsigned freq);
MinHeap* criarMinHeap(unsigned capacidade);
void minHeapify(MinHeap* minHeap, int idx);
MinHeapNode* extractMin(MinHeap* minHeap);
void insertMinHeap(MinHeap* minHeap, MinHeapNode* minHeapNode);
int isSizeOne(MinHeap* minHeap);
MinHeapNode* construirArvoreHuffman(unsigned char dados[], int freq[], int tamanho);
void gerarCodigos(MinHeapNode* raiz, int arr[], int top, char tabelaCodigos[256][256]);
void liberarArvore(MinHeapNode* raiz);

SequenciaInput* read_file(const char *filename, int *qtd_out) {
    FILE *fin = fopen(filename, "r");
    if (!fin) return NULL;

    int num_sequencias;
    if (fscanf(fin, "%d", &num_sequencias) != 1) {
        fclose(fin);
        return NULL;
    }

    *qtd_out = num_sequencias;
    SequenciaInput *lista = (SequenciaInput *)malloc(num_sequencias * sizeof(SequenciaInput));

    for (int i = 0; i < num_sequencias; i++) {
        lista[i].id = i;
        fscanf(fin, "%d", &lista[i].tamanho);
        lista[i].dados = (unsigned char *)malloc((lista[i].tamanho + 10) * sizeof(unsigned char));
        for (int k = 0; k < lista[i].tamanho; k++) {
            unsigned int temp_val;
            fscanf(fin, "%x", &temp_val);
            lista[i].dados[k] = (unsigned char)temp_val;
        }
    }

    fclose(fin);
    return lista;
}

void free_input(SequenciaInput *lista, int qtd) {
    if (!lista) return;
    for (int i = 0; i < qtd; i++) {
        free(lista[i].dados);
    }
    free(lista);
}

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
    for (int k = 0; k < 256; k++) {
        if (freq[k] > 0) {
            arr_chars[size] = (unsigned char)k;
            arr_freq[size] = freq[k];
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
    for (int k = 0; k < n; k++) {
        char *codigo = tabelaCodigos[dados[k]];
        int p = 0;
        while (codigo[p] != '\0') {
            adicionar_bit(&buffer, &bit_count, codigo[p], res.hex_string, &write_idx, &total_bytes);
            p++;
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
    int tam = minHeap->tamanho;
    if (dir < tam) {
        if (minHeap->array[dir]->freq < minHeap->array[menor]->freq) menor = dir;
    }
    if (esq < tam) {
        if (minHeap->array[esq]->freq < minHeap->array[menor]->freq) menor = esq;
    }
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

void gerarCodigos(MinHeapNode* raiz, int arr[], int top, char tabelaCodigos[256][256]) {
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
            int k = 0;
            while (k < top) {
                tabelaCodigos[raiz->dado][k] = arr[k] + '0';
                k++;
            }
            tabelaCodigos[raiz->dado][top] = '\0';
        }
    }
}

void liberarArvore(MinHeapNode* raiz) {
    if (raiz == NULL) return;
    liberarArvore(raiz->esq);
    liberarArvore(raiz->dir);
    free(raiz);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <entrada.txt> <saida.txt>\n", argv[0]);
        return 1;
    }
    int num_sequencias = 0;
    SequenciaInput *lista_sequencias = read_file(argv[1], &num_sequencias);
    if (!lista_sequencias) {
        printf("Erro ao ler arquivo ou arquivo vazio.\n");
        return 1;
    }
    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        free_input(lista_sequencias, num_sequencias);
        return 1;
    }
    for (int i = 0; i < num_sequencias; i++) {
        SequenciaInput atual = lista_sequencias[i];
        CompressaoResult rle = executar_RLE(atual.dados, atual.tamanho);
        CompressaoResult huf = executar_Huffman(atual.dados, atual.tamanho);
        if (huf.size_bytes == rle.size_bytes) {
            fprintf(fout, "%d->HUF(%.2f%%)=%s\n", atual.id, huf.taxa_compressao, huf.hex_string);
            fprintf(fout, "%d->RLE(%.2f%%)=%s\n", atual.id, rle.taxa_compressao, rle.hex_string);
        }
        else if (huf.size_bytes < rle.size_bytes) {
            fprintf(fout, "%d->HUF(%.2f%%)=%s\n", atual.id, huf.taxa_compressao, huf.hex_string);
        }
        else {
            fprintf(fout, "%d->RLE(%.2f%%)=%s\n", atual.id, rle.taxa_compressao, rle.hex_string);
        }
        free(rle.hex_string);
        free(huf.hex_string);
    }
    fclose(fout);
    free_input(lista_sequencias, num_sequencias);
    return 0;
}