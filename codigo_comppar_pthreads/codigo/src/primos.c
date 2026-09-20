/* primos.c - Parte 2, Opcao A: contagem de numeros primos em [1, K].
 *
 * Uso: ./primos <K> <num_threads>
 *
 * ESTRATEGIA DE DECOMPOSICAO
 * --------------------------
 * O custo do teste de primalidade cresce com o valor testado (divisoes ate
 * sqrt(n)). Uma particao estatica por blocos contiguos ([1,K/P], ...) faria
 * a ultima thread trabalhar varias vezes mais que a primeira -> severo
 * desbalanceamento de carga.
 *
 * Adotou-se DISTRIBUICAO CICLICA (round-robin de granularidade 1): a thread t
 * testa os numeros t, t+P, t+2P, ... Assim, numeros grandes e pequenos sao
 * distribuidos homogeneamente entre todas as threads.
 *
 * SINCRONIZACAO
 * -------------
 * Cada thread mantem um contador LOCAL; o acumulador global e atualizado uma
 * unica vez por thread, sob mutex (reducao). Evita-se contencao no laco.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long inicio;      /* primeiro numero testado por esta thread */
    long passo;       /* = numero total de threads (stride ciclico) */
    long limite;      /* K */
    long parciais;    /* contagem local desta thread */
} ArgThread;

static long total_primos = 0;                 /* acumulador global */
static pthread_mutex_t mutex_total = PTHREAD_MUTEX_INITIALIZER;

/* Teste de primalidade por divisao tentativa 6k+-1. */
static int eh_primo(long n) {
    if (n < 2)         return 0;
    if (n < 4)         return 1;   /* 2 e 3 */
    if (n % 2 == 0)    return 0;
    if (n % 3 == 0)    return 0;
    for (long d = 5; d * d <= n; d += 6) {
        if (n % d == 0 || n % (d + 2) == 0) return 0;
    }
    return 1;
}

static void *trabalhador(void *arg) {
    ArgThread *a = (ArgThread *)arg;
    long local = 0;

    /* Distribuicao ciclica: balanceamento natural da carga. */
    for (long n = a->inicio; n <= a->limite; n += a->passo) {
        if (eh_primo(n)) local++;
    }
    a->parciais = local;

    /* Reducao: uma unica secao critica por thread. */
    pthread_mutex_lock(&mutex_total);
    total_primos += local;
    pthread_mutex_unlock(&mutex_total);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <K> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long K = strtol(argv[1], NULL, 10);
    int  P = (int)strtol(argv[2], NULL, 10);
    if (K < 1 || P < 1) {
        fprintf(stderr, "Erro: K e num_threads devem ser >= 1.\n");
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

    for (int t = 0; t < P; t++) {
        args[t].inicio   = 2 + t;   /* comeca em 2, deslocado pelo id */
        args[t].passo    = P;
        args[t].limite   = K;
        args[t].parciais = 0;
        if (pthread_create(&threads[t], NULL, trabalhador, &args[t]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }
    for (int t = 0; t < P; t++) {
        pthread_join(threads[t], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Primos] K=%ld | threads=%d | primos=%ld | Tempo: %.4f s\n",
           K, P, total_primos, tempo);

    /* Evidencia de balanceamento: contribuicao de cada thread. */
    for (int t = 0; t < P; t++) {
        printf("   thread %d -> %ld primos\n", t, args[t].parciais);
    }

    free(threads);
    free(args);
    pthread_mutex_destroy(&mutex_total);
    return 0;
}
