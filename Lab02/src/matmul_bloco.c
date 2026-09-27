/*
 * matmul_bloco.c
 * CompPar - Item 3: Multiplicacao de Matrizes com Blocagem (Tiling)
 *
 * C = A x B, particionada em blocos B x B, laco interno i,k,j para manter
 * acesso contiguo (Row-Major) em B[k*N+j] e C[i*N+j], favorecendo SIMD.
 *
 * Uso: ./matmul_bloco N TAM_BLOCO
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
    if (argc != 3) {
        fprintf(stderr, "Uso: %s N TAM_BLOCO\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int Bsz = atoi(argv[2]);
    if (N <= 0 || Bsz <= 0) {
        fprintf(stderr, "N e TAM_BLOCO devem ser positivos.\n");
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

    for (int ii = 0; ii < N; ii += Bsz) {
        for (int jj = 0; jj < N; jj += Bsz) {
            for (int kk = 0; kk < N; kk += Bsz) {
                int i_max = (ii + Bsz < N) ? ii + Bsz : N;
                int k_max = (kk + Bsz < N) ? kk + Bsz : N;
                int j_max = (jj + Bsz < N) ? jj + Bsz : N;
                for (int i = ii; i < i_max; i++) {
                    for (int k = kk; k < k_max; k++) {
                        double r = A[i * N + k];
                        for (int j = jj; j < j_max; j++) {
                            C[i * N + j] += r * B[k * N + j];
                        }
                    }
                }
            }
        }
    }

    double t1 = now_sec();
    double tempo = t1 - t0;
    double gflops = (2.0 * (double)N * (double)N * (double)N) / (tempo * 1e9);

    double checksum = 0.0;
    for (size_t idx = 0; idx < elems; idx++) checksum += C[idx];

    printf("[Matmul Blocado] N: %d | Bloco B: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.4f\n",
           N, Bsz, tempo, gflops, checksum);

    free(A); free(B); free(C);
    return 0;
}
