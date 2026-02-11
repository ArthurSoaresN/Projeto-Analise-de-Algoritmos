#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUE 100

typedef struct {
    int id;
    int pos_init_line;
    int pos_init_colum;
    int linhas;
    int colunas;
    int matriz[MAX_VALUE][MAX_VALUE];
} mapa;

int read_file(const char* filename, mapa** lista_matrizes) {

    FILE* file = fopen(filename, "r");

    int n_linhas, n_colunas;
    int n_matrizes = 0;

    // matriz[i][j]

    fscanf(file, "%d\n", &n_matrizes);

    *lista_matrizes = (mapa*)malloc(n_matrizes * sizeof(mapa));

    for (int m = 0; m < n_matrizes; m++) {
        fscanf(file, "%d %d", &n_colunas, &n_linhas);

        (*lista_matrizes)[m].id = m;
        (*lista_matrizes)[m].linhas = n_linhas;
        (*lista_matrizes)[m].colunas = n_colunas;

        for (int i = 0; i < n_linhas; i++) {
            for (int j = 0; j < n_colunas; j++) {
                char buffer[10];
                fscanf(file, "%s", buffer);

                if (buffer[0] == 'X') {
                    (*lista_matrizes)[m].pos_init_line = i;
                    (*lista_matrizes)[m].pos_init_colum = j;
                    (*lista_matrizes)[m].matriz[i][j] = 0;
                } else {
                    (*lista_matrizes)[m].matriz[i][j] = atoi(buffer);
                }
            }
        }
    }

    fclose(file);
    return n_matrizes;
}

// Retorna 1 se encontrou a saída, 0 se deu em beco sem saída
int explorar(FILE* output, mapa* m, int r, int c, int pr, int pc, int nL, int nC) {
    
    // Se está na borda e NÃO é a posição inicial 'X'
    if ((r == 0 || r == nL - 1 || c == 0 || c == nC - 1) && 
        (r != m->pos_init_line || c != m->pos_init_colum)) {
        fprintf(output, "|FIM@%d,%d\n", r, c);
        return 1; 
    }

    // TENTAR DIREITA (linha, coluna + 1)
    int dr = r, dc = c + 1;
    if (dc < nC && m->matriz[dr][dc] == 0) {
        fprintf(output, "|D->%d,%d", dr, dc);
        m->matriz[dr][dc] = -1; // Marca como visitado
        if (explorar(output, m, dr, dc, r, c, nL, nC)) return 1;
    }

    // TENTAR FRENTE (linha - 1, coluna)
    int fr = r - 1, fc = c;
    if (fr >= 0 && m->matriz[fr][fc] == 0) {
        fprintf(output, "|F->%d,%d", fr, fc);
        m->matriz[fr][fc] = -1;
        if (explorar(output, m, fr, fc, r, c, nL, nC)) return 1;
    }

    // TENTAR ESQUERDA (linha, coluna - 1)
    int er = r, ec = c - 1;
    if (ec >= 0 && m->matriz[er][ec] == 0) {
        fprintf(output, "|E->%d,%d", er, ec);
        m->matriz[er][ec] = -1;
        if (explorar(output, m, er, ec, r, c, nL, nC)) return 1;
    }

    // TENTAR TRÁS (linha + 1, coluna)
    int tr = r + 1, tc = c;
    if (tr < nL && m->matriz[tr][tc] == 0) {
        fprintf(output, "|T->%d,%d", tr, tc);
        m->matriz[tr][tc] = -1;
        if (explorar(output, m, tr, tc, r, c, nL, nC)) return 1;
    }

    // 6. BACKTRACKING
    // Só imprime BT se não estivermos no INI voltando para o "nada"
    if (pr != -1) {
        fprintf(output, "|BT@%d,%d->%d,%d", r, c, pr, pc);
    }
    return 0;
}

void processar_matrizes (FILE* output, mapa* lista_matrizes, int n_matrizes) {

    for (int m = 0; m < n_matrizes; m++) {

        int line = lista_matrizes[m].pos_init_line; // r
        int colum = lista_matrizes[m].pos_init_colum; // c

        int nL = lista_matrizes[m].linhas;
        int nC = lista_matrizes[m].colunas;

        fprintf(output, "L%d:INI@%d,%d", m, line, colum);

        lista_matrizes[m].matriz[line][colum] = -1;

        if (!explorar(output, &lista_matrizes[m], line, colum, -1, -1, nL, nC)) {
            fprintf(output, "|FIM@-,-\n");
        }
    }
}

int main(int argc, char* argv[]) { 

    if(argc < 3) return 1;

    mapa* lista_matrizes = NULL;

    int n_matrizes = read_file(argv[1], &lista_matrizes);

    FILE* output = fopen(argv[2], "w");

    if (!output) return 2;

    processar_matrizes(output, lista_matrizes, n_matrizes);

    fclose(output);
    free(lista_matrizes);

    return 0;
}

