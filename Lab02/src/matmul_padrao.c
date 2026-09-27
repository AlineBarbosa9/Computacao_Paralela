/*
 * matmul_padrao.c
 * CompPar - Item 3: Multiplicacao de Matrizes Sequencial (Algoritmo Canonico)
 *
 * C = A x B, laco i,j,k. B[k*N+j] e acessado saltando linhas -> muitos cache misses.
 *
 * Uso: ./matmul_padrao N
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

    size_t elems = (size_t)N * (size_t)N;
    double *A = malloc(elems * sizeof(double));
    double *B = malloc(elems * sizeof(double));
    double *C = calloc(elems, sizeof(double));
    if (!A || !B || !C) {
        fprintf(stderr, "Falha de alocacao para N=%d\n", N);
        return 1;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = (double)(i + j);
            B[i * N + j] = (double)(i * j);
        }
    }

    double t0 = now_sec();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double soma = 0.0;
            for (int k = 0; k < N; k++) {
                soma += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = soma;
        }
    }

    double t1 = now_sec();
    double tempo = t1 - t0;
    double gflops = (2.0 * (double)N * (double)N * (double)N) / (tempo * 1e9);

    /* checksum evita que o compilador elimine o calculo e serve de verificacao */
    double checksum = 0.0;
    for (size_t idx = 0; idx < elems; idx++) checksum += C[idx];

    printf("[Matmul Padrao] N: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.4f\n",
           N, tempo, gflops, checksum);

    free(A); free(B); free(C);
    return 0;
}
