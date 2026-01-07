#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// ESTRUTURAS
// ============================================================================

struct MinHeapNode {
    unsigned char dado;
    unsigned freq;
    struct MinHeapNode *esq, *dir;
};

struct MinHeap {
    unsigned tamanho;
    unsigned capacidade;
    struct MinHeapNode** array;
};

typedef struct {
    int size;
    char *hex_code;
    float taxa;
} Result;

// ============================================================================
// FUNÇÕES AUXILIARES HEAP / HUFFMAN
// ============================================================================

struct MinHeapNode* novoNo(unsigned char dado, unsigned freq) {
    struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
    temp->esq = temp->dir = NULL;
    temp->dado = dado;
    temp->freq = freq;
    return temp;
}

struct MinHeap* createMinHeap(unsigned capacidade) {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->tamanho = 0;
    minHeap->capacidade = capacidade;
    minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacidade * sizeof(struct MinHeapNode*));
    return minHeap;
}

void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b) {
    struct MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

// Critério de desempate: Frequência > Valor do Byte (Dado)
void minHeapify(struct MinHeap* minHeap, int idx) {
    int menor = idx;
    int esq = 2 * idx + 1;
    int dir = 2 * idx + 2;

    if (esq < minHeap->tamanho) {
        if (minHeap->array[esq]->freq < minHeap->array[menor]->freq ||
           (minHeap->array[esq]->freq == minHeap->array[menor]->freq && minHeap->array[esq]->dado < minHeap->array[menor]->dado)) {
            menor = esq;
        }
    }

    if (dir < minHeap->tamanho) {
        if (minHeap->array[dir]->freq < minHeap->array[menor]->freq ||
           (minHeap->array[dir]->freq == minHeap->array[menor]->freq && minHeap->array[dir]->dado < minHeap->array[menor]->dado)) {
            menor = dir;
        }
    }

    if (menor != idx) {
        swapMinHeapNode(&minHeap->array[menor], &minHeap->array[idx]);
        minHeapify(minHeap, menor);
    }
}

struct MinHeapNode* extractMin(struct MinHeap* minHeap) {
    struct MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->tamanho - 1];
    --minHeap->tamanho;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode) {
    ++minHeap->tamanho;
    int i = minHeap->tamanho - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

int isSizeOne(struct MinHeap* minHeap) {
    return (minHeap->tamanho == 1);
}

// --- CONSTRUÇÃO DA ÁRVORE (CORRIGIDA - CRASH FIX) ---
struct MinHeapNode* construirArvoreHuffman(unsigned char dados[], int freq[], int size) {
    struct MinHeapNode *esq, *dir, *top;
    struct MinHeap* minHeap = createMinHeap(size);

    // Passo 1: Preencher o array diretamente
    for (int i = 0; i < size; ++i) {
        minHeap->array[i] = novoNo(dados[i], freq[i]);
    }
    minHeap->tamanho = size;

    // Passo 2: Construir o Heap de baixo para cima (Heapify Reverso)
    // FIX: Verificar se size > 1 para evitar Underflow de unsigned int (1 - 2 = CRASH)
    if (size > 1) {
        for (int i = (minHeap->tamanho - 2) / 2; i >= 0; --i) {
            minHeapify(minHeap, i);
        }
    }

    // Passo 3: Construir a árvore
    while (!isSizeOne(minHeap)) {
        esq = extractMin(minHeap);
        dir = extractMin(minHeap);
        // Nó pai recebe '$' mas o desempate real ocorre nos nós folha durante o Heapify
        top = novoNo('$', esq->freq + dir->freq);
        top->esq = esq;
        top->dir = dir;
        insertMinHeap(minHeap, top);
    }
    
    struct MinHeapNode* raiz = extractMin(minHeap);
    free(minHeap->array); // Limpeza básica
    free(minHeap);
    
    return raiz;
}

void armazenarCodigos(struct MinHeapNode* raiz, int arr[], int top, char** codigos) {
    if (raiz->esq) {
        arr[top] = 0;
        armazenarCodigos(raiz->esq, arr, top + 1, codigos);
    }
    if (raiz->dir) {
        arr[top] = 1;
        armazenarCodigos(raiz->dir, arr, top + 1, codigos);
    }
    if (!(raiz->esq) && !(raiz->dir)) {
        codigos[raiz->dado] = (char*)malloc((top + 1) * sizeof(char));
        for (int i = 0; i < top; i++) {
            codigos[raiz->dado][i] = arr[i] + '0';
        }
        codigos[raiz->dado][top] = '\0';
    }
}

// ============================================================================
// PROCESSAMENTO HUFFMAN
// ============================================================================

Result processarHuffman(unsigned char* buffer, int n) {
    Result res;
    if (n == 0) {
        res.size = 0;
        res.hex_code = strdup("");
        res.taxa = 0.0;
        return res;
    }

    // Contagem de frequência
    int freq[256] = {0};
    for (int i = 0; i < n; i++) freq[buffer[i]]++;

    unsigned char dados[256];
    int freqs[256];
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            dados[size] = (unsigned char)i;
            freqs[size] = freq[i];
            size++;
        }
    }

    struct MinHeapNode* raiz = construirArvoreHuffman(dados, freqs, size);
    
    char* codigos[256] = {0};
    int arr[100], top = 0;
    armazenarCodigos(raiz, arr, top, codigos);
    
    // Tratamento especial para apenas 1 símbolo único (ex: AAAAA) -> bit 0
    if (size == 1) {
         if (codigos[dados[0]]) free(codigos[dados[0]]); // Libera alocação vazia se existir
         codigos[dados[0]] = strdup("0");
    }

    // Geração da String Hexadecimal
    res.hex_code = (char*)malloc(n * 2 + 100); 
    int hex_idx = 0;
    unsigned char bit_buffer = 0;
    int bit_count = 0;
    int total_bytes = 0;

    for (int i = 0; i < n; i++) {
        char* code = codigos[buffer[i]];
        if (!code) continue; // Segurança
        for (int j = 0; code[j]; j++) {
            if (code[j] == '1') bit_buffer |= (1 << (7 - bit_count));
            bit_count++;
            if (bit_count == 8) {
                hex_idx += sprintf(res.hex_code + hex_idx, "%02X", bit_buffer);
                total_bytes++;
                bit_buffer = 0;
                bit_count = 0;
            }
        }
    }
    if (bit_count > 0) {
        hex_idx += sprintf(res.hex_code + hex_idx, "%02X", bit_buffer);
        total_bytes++;
    }

    res.size = total_bytes;
    res.taxa = ((float)res.size / n) * 100;
    
    // Limpeza dos códigos
    for(int i=0; i<256; i++) if(codigos[i]) free(codigos[i]);
    // Idealmente liberaria a árvore 'raiz' recursivamente aqui
    
    return res;
}

// ============================================================================
// PROCESSAMENTO RLE
// ============================================================================

Result processarRLE(unsigned char* buffer, int n) {
    Result res;
    if (n == 0) {
        res.size = 0;
        res.hex_code = strdup("");
        res.taxa = 0.0;
        return res;
    }
    
    res.hex_code = (char*)malloc(n * 2 * 2 + 100);
    int hex_idx = 0;
    int size = 0;
    
    for (int i = 0; i < n; i++) {
        unsigned char atual = buffer[i];
        int count = 1;
        while (i + 1 < n && buffer[i+1] == atual && count < 255) {
            count++;
            i++;
        }
        hex_idx += sprintf(res.hex_code + hex_idx, "%02X%02X", count, atual);
        size += 2;
    }
    
    res.size = size;
    res.taxa = ((float)size / n) * 100;
    return res;
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char *argv[]) {
    // Verifica se os arquivos foram passados
    if (argc < 3) {
        printf("Uso: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        printf("Erro ao abrir entrada.\n");
        return 1;
    }

    FILE *fout = fopen(argv[2], "w");
    if (!fout) { 
        printf("Erro ao criar saida.\n");
        fclose(fin); 
        return 1; 
    }

    int num_casos;
    if (fscanf(fin, "%d", &num_casos) != 1) {
        fclose(fin); fclose(fout); return 0;
    }

    for (int k = 0; k < num_casos; k++) {
        int n;
        fscanf(fin, "%d", &n);
        
        unsigned char *buffer = (unsigned char*)malloc(n);
        for (int i = 0; i < n; i++) {
            unsigned int temp;
            fscanf(fin, "%x", &temp);
            buffer[i] = (unsigned char)temp;
        }

        Result huf = processarHuffman(buffer, n);
        Result rle = processarRLE(buffer, n);

        fprintf(fout, "%d->HUF(%.2f%%)=%s\n", k, huf.taxa, huf.hex_code);
        
        if (rle.size < huf.size) {
             fprintf(fout, "%d->RLE(%.2f%%)=%s\n", k, rle.taxa, rle.hex_code);
        } else if (rle.size == huf.size) {
             fprintf(fout, "%d->RLE(%.2f%%)=%s\n", k, rle.taxa, rle.hex_code);
        }

        free(buffer);
        free(huf.hex_code);
        free(rle.hex_code);
    }

    fclose(fin);
    fclose(fout);
    return 0;
}