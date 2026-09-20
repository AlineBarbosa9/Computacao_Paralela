/* soma_vetores.c - Parte 2, Opcao C: C[i] = A[i] + B[i] para vetores gigantes.
 *
 * Uso: ./soma_vetores <N> <num_threads>
 *
 * ESTRATEGIA
 * ----------
 * Particionamento estatico por BLOCOS CONTIGUOS de indices. Cada thread
 * percorre uma faixa [ini, fim) sequencialmente, o que favorece a localidade
 * espacial e o prefetch de hardware (linhas de cache de 64 B).
 *
 * FALSA PARTILHA
 * --------------
 * Como os blocos sao contiguos e grandes, apenas as duas linhas de cache das
 * fronteiras sao compartilhadas -> efeito desprezivel. Nao ha escrita
 * compartilhada e, portanto, nenhum mutex e necessario no laco.
 *
 * NATUREZA DO PROBLEMA
 * --------------------
 * 1 soma para cada 24 bytes movimentados (2 leituras + 1 escrita de double):
 * intensidade aritmetica baixissima -> problema MEMORY-BOUND. O speedup
 * satura assim que a largura de banda da DRAM e atingida.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    const double *A;
    const double *B;
    double *C;
    long ini;   /* inclusivo */
    long fim;   /* exclusivo */
} ArgThread;

static void *trabalhador(void *arg) {
    ArgThread *a = (ArgThread *)arg;
    for (long i = a->ini; i < a->fim; i++) {
        a->C[i] = a->A[i] + a->B[i];
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <N> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long N = strtol(argv[1], NULL, 10);
    int  P = (int)strtol(argv[2], NULL, 10);
    if (N < 1 || P < 1) {
        fprintf(stderr, "Erro: N e num_threads devem ser >= 1.\n");
        return EXIT_FAILURE;
    }

    double *A = malloc((size_t)N * sizeof(double));
    double *B = malloc((size_t)N * sizeof(double));
    double *C = malloc((size_t)N * sizeof(double));
    if (!A || !B || !C) {
        fprintf(stderr, "Erro: falha ao alocar %.2f GB.\n",
                3.0 * (double)N * sizeof(double) / 1e9);
        return EXIT_FAILURE;
    }

    /* Inicializacao (fracao sequencial do programa - relevante p/ Amdahl). */
    for (long i = 0; i < N; i++) {
        A[i] = (double)i * 0.5;
        B[i] = (double)i * 1.5;
    }

    pthread_t *threads = malloc((size_t)P * sizeof(pthread_t));
    ArgThread *args    = malloc((size_t)P * sizeof(ArgThread));
    if (!threads || !args) { perror("malloc"); return EXIT_FAILURE; }

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio); /* mede apenas a soma paralela */

    long base  = N / P;
    long resto = N % P;
    long cursor = 0;
    for (int t = 0; t < P; t++) {
        long tam = base + (t < resto ? 1 : 0);
        args[t].A = A; args[t].B = B; args[t].C = C;
        args[t].ini = cursor;
        args[t].fim = cursor + tam;
        cursor += tam;
        if (pthread_create(&threads[t], NULL, trabalhador, &args[t]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }
    for (int t = 0; t < P; t++) pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    /* Verificacao de corretude em pontos amostrais. */
    int ok = 1;
    long amostras[3] = {0, N / 2, N - 1};
    for (int k = 0; k < 3; k++) {
        long i = amostras[k];
        if (C[i] != A[i] + B[i]) ok = 0;
    }

    /* Largura de banda efetiva: 3 acessos de 8 bytes por elemento. */
    double gbs = (3.0 * (double)N * sizeof(double)) / tempo / 1e9;

    printf("[Soma Vetores] N=%ld | threads=%d | verificacao=%s | "
           "Tempo: %.4f s | Banda efetiva: %.2f GB/s\n",
           N, P, ok ? "OK" : "FALHOU", tempo, gbs);

    free(A); free(B); free(C); free(threads); free(args);
    return 0;
}
