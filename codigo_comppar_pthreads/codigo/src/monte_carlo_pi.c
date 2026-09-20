/* monte_carlo_pi.c - Parte 2, Opcao B: estimativa de pi por Monte Carlo.
 *
 * Uso: ./monte_carlo_pi <N> <num_threads>
 *
 * ESTRATEGIA
 * ----------
 * Os N sorteios sao divididos estaticamente entre as P threads (carga
 * homogenea: cada ponto custa o mesmo). O resto da divisao e distribuido
 * entre as primeiras threads.
 *
 * REENTRANCIA (ponto critico da Opcao B)
 * --------------------------------------
 * rand() mantem estado global protegido internamente na glibc, o que
 * serializaria todas as threads. Usa-se rand_r(&semente), em que cada thread
 * possui sua propria semente na PILHA, eliminando estado compartilhado.
 * As sementes sao distintas por thread para garantir fluxos independentes.
 *
 * SINCRONIZACAO
 * -------------
 * Cada thread devolve sua contagem local; o total e obtido por reducao apos
 * os joins (sem mutex algum no laco).
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long   n_pontos;     /* quantidade de sorteios desta thread */
    unsigned int semente;/* semente privada para rand_r */
    long   dentro;       /* resultado parcial */
} ArgThread;

static void *trabalhador(void *arg) {
    ArgThread *a = (ArgThread *)arg;
    unsigned int semente = a->semente; /* copia local (registrador/pilha) */
    long dentro = 0;

    for (long i = 0; i < a->n_pontos; i++) {
        double x = (double)rand_r(&semente) / (double)RAND_MAX;
        double y = (double)rand_r(&semente) / (double)RAND_MAX;
        if (x * x + y * y <= 1.0) dentro++;
    }
    a->dentro = dentro;
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

    pthread_t *threads = malloc((size_t)P * sizeof(pthread_t));
    ArgThread *args    = malloc((size_t)P * sizeof(ArgThread));
    if (!threads || !args) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    long base  = N / P;
    long resto = N % P;
    for (int t = 0; t < P; t++) {
        args[t].n_pontos = base + (t < resto ? 1 : 0);
        /* Sementes distintas e reprodutiveis por thread. */
        args[t].semente  = (unsigned int)(12345u + 7919u * (unsigned int)t);
        args[t].dentro   = 0;
        if (pthread_create(&threads[t], NULL, trabalhador, &args[t]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    long total_dentro = 0;
    for (int t = 0; t < P; t++) {
        pthread_join(threads[t], NULL);
        total_dentro += args[t].dentro; /* reducao apos o join */
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    double pi = 4.0 * (double)total_dentro / (double)N;
    printf("[Monte Carlo] N=%ld | threads=%d | pi=%.8f | dentro=%ld | Tempo: %.4f s\n",
           N, P, pi, total_dentro, tempo);

    free(threads);
    free(args);
    return 0;
}
