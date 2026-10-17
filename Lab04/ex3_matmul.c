/*
 * Nome: Aline Barbosa Vidal RA: 10721348
 * CompPar - Lab Explorando-OpenMP - Exercício 3
 * Multiplicação de matrizes C = A * B (double, N x N) com bom uso de cache.
 *
 * Técnicas:
 *  - Ordem de laços i-k-j: o laço mais interno percorre B[k][*] e C[i][*]
 *    com passo 1 (acesso contíguo, vetorizável) em vez de B[*][j] (passo N).
 *  - Blocking/tiling (BS x BS): os três blocos (A, B, C) usados no miolo cabem
 *    no cache L1/L2. BS=32 -> 3 * 32*32*8 B = 24 KB (cabe em L1 de 32 KB).
 *  - Paralelo: #pragma omp parallel for collapse(2) sobre os blocos (ii,jj).
 *    Cada thread é dona exclusiva de blocos de C -> sem condição de corrida e
 *    sem false sharing relevante.
 *
 * Compilar: gcc -O2 -fopenmp ex3_matmul.c -o ex3_matmul   (ajuste: -DBS=64)
 * Executar: ./ex3_matmul <seq|omp> <threads> <N> <repeticoes>
 * Saída (CSV): versao,threads,N,rep,tempo_s
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#ifndef BS
#define BS 32
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void matmul_seq(const double *A, const double *B, double *C, int N) {
    for (int ii = 0; ii < N; ii += BS)
        for (int jj = 0; jj < N; jj += BS)
            for (int kk = 0; kk < N; kk += BS)
                for (int i = ii; i < MIN(ii + BS, N); i++)
                    for (int k = kk; k < MIN(kk + BS, N); k++) {
                        double aik = A[(size_t)i * N + k];
                        for (int j = jj; j < MIN(jj + BS, N); j++)
                            C[(size_t)i * N + j] += aik * B[(size_t)k * N + j];
                    }
}

static void matmul_omp(const double *A, const double *B, double *C, int N, int threads) {
#pragma omp parallel for collapse(2) schedule(static) num_threads(threads)
    for (int ii = 0; ii < N; ii += BS)
        for (int jj = 0; jj < N; jj += BS)
            for (int kk = 0; kk < N; kk += BS)
                for (int i = ii; i < MIN(ii + BS, N); i++)
                    for (int k = kk; k < MIN(kk + BS, N); k++) {
                        double aik = A[(size_t)i * N + k];
                        for (int j = jj; j < MIN(jj + BS, N); j++)
                            C[(size_t)i * N + j] += aik * B[(size_t)k * N + j];
                    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <seq|omp> <threads> <N> <repeticoes>\n", argv[0]);
        return 1;
    }
    int par = strcmp(argv[1], "omp") == 0;
    int threads = par ? atoi(argv[2]) : 1;
    int N = atoi(argv[3]);
    int reps = atoi(argv[4]);

    size_t bytes = (size_t)N * N * sizeof(double);
    double *A = malloc(bytes), *B = malloc(bytes), *C = malloc(bytes);
    if (!A || !B || !C) { fprintf(stderr, "Sem memoria\n"); return 1; }

    /* Valores inteiros pequenos, determinísticos -> somas exatas em double */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[(size_t)i * N + j] = (double)((i * 7 + j * 3) % 10);
            B[(size_t)i * N + j] = (double)((i * 5 + j * 11) % 10);
        }

    /* Checksum esperado em O(N^2): sum(C) = sum_k (sum_i A[i][k]) * (sum_j B[k][j]) */
    double esperado = 0.0;
    for (int k = 0; k < N; k++) {
        double ca = 0.0, rb = 0.0;
        for (int i = 0; i < N; i++) ca += A[(size_t)i * N + k];
        for (int j = 0; j < N; j++) rb += B[(size_t)k * N + j];
        esperado += ca * rb;
    }

    for (int rep = 1; rep <= reps; rep++) {
        memset(C, 0, bytes);
        double t0 = omp_get_wtime();
        if (par) matmul_omp(A, B, C, N, threads);
        else     matmul_seq(A, B, C, N);
        double t1 = omp_get_wtime();

        double soma = 0.0;
        for (size_t i = 0; i < (size_t)N * N; i++) soma += C[i];
        if (soma != esperado) {
            fprintf(stderr, "ERRO: checksum %.0f != esperado %.0f\n", soma, esperado);
            return 2;
        }
        printf("%s,%d,%d,%d,%.6f\n", par ? "omp" : "seq", threads, N, rep, t1 - t0);
    }
    fprintf(stderr, "[%s] threads=%d N=%d BS=%d checksum OK\n", par ? "omp" : "seq", threads, N, BS);
    free(A); free(B); free(C);
    return 0;
}
