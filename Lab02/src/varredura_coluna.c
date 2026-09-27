/*
 * varredura_coluna.c
 * CompPar - Item 2: Impacto da Localidade Espacial (Linha vs. Coluna)
 *
 * Varredura em ordem de COLUNA (laco externo em j, laco interno em i).
 * Acessa A[i*N+j] saltando N*8 bytes a cada iteracao -> pobre localidade espacial.
 *
 * Uso: ./varredura_coluna N
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s N\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "N deve ser positivo.\n");
        return 1;
    }

    double *A = malloc((size_t)N * (size_t)N * sizeof(double));
    if (!A) {
        fprintf(stderr, "Falha de alocacao para N=%d\n", N);
        return 1;
    }

    /* Inicializacao deterministica padronizada (mesma ordem de escrita, linha a linha) */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = (double)(i + j);
        }
    }

    double t0 = now_sec();

    long pares = 0;
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            if ((int)A[i * N + j] % 2 == 0) {
                pares++;
            }
        }
    }

    double t1 = now_sec();

    printf("[Varredura Coluna] N: %d | Pares: %ld | Tempo: %.6f s\n", N, pares, t1 - t0);

    free(A);
    return 0;
}
