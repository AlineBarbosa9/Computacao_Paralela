/*
 * matmul_pthreads.c
 * CompPar - Item 4: Adicionando Paralelismo com Pthreads
 *
 * C = A x B particionada por FAIXAS CONTIGUAS DE LINHAS entre T threads.
 * Cada thread escreve exclusivamente em suas proprias linhas de C, portanto
 * nao ha necessidade de mutex (regiao 100% livre de contencao) e a fronteira
 * de compartilhamento entre threads se resume a, no maximo, uma linha de
 * cache no limiar do bloco (mitigacao de False Sharing).
 *
 * Uso: ./matmul_pthreads N NUM_THREADS
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    const double *A;
    const double *B;
    double *C;
    int N;
    int linha_inicio;
    int linha_fim; /* exclusivo */
} ThreadArg;

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void *worker(void *arg_ptr) {
    ThreadArg *arg = (ThreadArg *)arg_ptr;
    int N = arg->N;
    for (int i = arg->linha_inicio; i < arg->linha_fim; i++) {
        for (int j = 0; j < N; j++) {
            double soma = 0.0;
            for (int k = 0; k < N; k++) {
                soma += arg->A[i * N + k] * arg->B[k * N + j];
            }
            arg->C[i * N + j] = soma;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s N NUM_THREADS\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int T = atoi(argv[2]);
    if (N <= 0 || T <= 0) {
        fprintf(stderr, "N e NUM_THREADS devem ser positivos.\n");
        return 1;
    }
    if (T > N) T = N; /* nao faz sentido mais threads que linhas */

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

    pthread_t *threads = malloc((size_t)T * sizeof(pthread_t));
    ThreadArg *args = malloc((size_t)T * sizeof(ThreadArg));

    int linhas_por_thread = N / T;
    int resto = N % T;

    double t0 = now_sec();

    int linha_atual = 0;
    for (int t = 0; t < T; t++) {
        int extra = (t < resto) ? 1 : 0; /* distribui o resto entre as primeiras threads */
        int inicio = linha_atual;
        int fim = inicio + linhas_por_thread + extra;
        args[t] = (ThreadArg){ .A = A, .B = B, .C = C, .N = N,
                                .linha_inicio = inicio, .linha_fim = fim };
        linha_atual = fim;
        pthread_create(&threads[t], NULL, worker, &args[t]);
    }
    for (int t = 0; t < T; t++) {
        pthread_join(threads[t], NULL);
    }

    double t1 = now_sec();
    double tempo = t1 - t0;
    double gflops = (2.0 * (double)N * (double)N * (double)N) / (tempo * 1e9);

    double checksum = 0.0;
    for (size_t idx = 0; idx < elems; idx++) checksum += C[idx];

    printf("[Matmul Pthreads] N: %d | Threads: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.4f\n",
           N, T, tempo, gflops, checksum);

    free(A); free(B); free(C); free(threads); free(args);
    return 0;
}
