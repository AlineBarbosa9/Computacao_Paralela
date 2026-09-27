/*
 * matmul_pthreads_bloco.c
 * CompPar - Item 4: Pthreads + Blocagem (Tiling)
 *
 * Cada thread recebe uma faixa contigua de linhas de C (mesma estrategia de
 * particionamento de matmul_pthreads.c, livre de mutex e com fronteira de
 * False Sharing limitada a uma linha de cache) e, dentro de sua faixa,
 * executa a multiplicacao usando sub-blocos B x B para maximizar a
 * reutilizacao de dados nos caches L1/L2.
 *
 * Uso: ./matmul_pthreads_bloco N NUM_THREADS TAM_BLOCO
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
    int Bsz;
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
    int N = arg->N, Bsz = arg->Bsz;
    const double *A = arg->A, *B = arg->B;
    double *C = arg->C;

    for (int ii = arg->linha_inicio; ii < arg->linha_fim; ii += Bsz) {
        int i_max = (ii + Bsz < arg->linha_fim) ? ii + Bsz : arg->linha_fim;
        for (int jj = 0; jj < N; jj += Bsz) {
            int j_max = (jj + Bsz < N) ? jj + Bsz : N;
            for (int kk = 0; kk < N; kk += Bsz) {
                int k_max = (kk + Bsz < N) ? kk + Bsz : N;
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
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s N NUM_THREADS TAM_BLOCO\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int T = atoi(argv[2]);
    int Bsz = atoi(argv[3]);
    if (N <= 0 || T <= 0 || Bsz <= 0) {
        fprintf(stderr, "N, NUM_THREADS e TAM_BLOCO devem ser positivos.\n");
        return 1;
    }
    if (T > N) T = N;

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
        int extra = (t < resto) ? 1 : 0;
        int inicio = linha_atual;
        int fim = inicio + linhas_por_thread + extra;
        args[t] = (ThreadArg){ .A = A, .B = B, .C = C, .N = N, .Bsz = Bsz,
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

    printf("[Matmul Pthreads+Bloco] N: %d | Threads: %d | Bloco B: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.4f\n",
           N, T, Bsz, tempo, gflops, checksum);

    free(A); free(B); free(C); free(threads); free(args);
    return 0;
}
