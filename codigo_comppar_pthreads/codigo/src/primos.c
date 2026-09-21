/* primos.c - Parte 2, Opcao A: contagem de numeros primos em [1, K].
 *
 * Uso: ./primos <K> <num_threads>
 *
 * ESTRATEGIA DE DECOMPOSICAO
 * --------------------------
 * O custo do teste de primalidade cresce com o valor testado (divisoes ate
 * sqrt(n)). Uma particao estatica por blocos contiguos ([1,K/N],
 * [K/N + 1, 2K/N], etc.) gera severo desbalanceamento de carga.
 *
 * Uma primeira versao com distribuicao ciclica de granularidade 1 (thread t
 * testa t, t+P, t+2P, ...) revelou um defeito sutil: quando P e par (2, 4, 8
 * - exatamente os casos exigidos pelo laboratorio), cada thread fica presa a
 * uma unica classe de paridade. Threads de deslocamento par passam a testar
 * somente numeros pares, dos quais apenas o 2 e primo, enquanto as threads
 * de deslocamento impar concentram quase todo o trabalho real. A contagem
 * total permanecia correta, mas o balanceamento de carga - objetivo central
 * desta questao - ficava quebrado.
 *
 * A solucao adotada e BLOCOS DINAMICOS: um contador atomico compartilhado
 * (proximo_numero) distribui, sob demanda, blocos contiguos de TAM_BLOCO
 * numeros a quem pedir. Uma thread que processa blocos "faceis" (regioes com
 * poucos primos, rejeitados cedo) simplesmente pede o proximo bloco mais
 * cedo, autoequilibrando a carga sem qualquer correlacao com paridade ou
 * outra estrutura aritmetica dos numeros testados.
 *
 * SINCRONIZACAO
 * -------------
 * A distribuicao de blocos usa uma unica variavel atomica (sem mutex,
 * portanto sem risco de contencao de lock nem de chamada de sistema). Cada
 * thread mantem um contador LOCAL de primos; o acumulador global e
 * atualizado uma unica vez por thread, sob mutex (reducao classica), ao
 * final.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
 
#define TAM_BLOCO 10000L /* granularidade do trabalho dinamico */
 
typedef struct {
    long limite;      /* K */
    long parciais;    /* contagem local desta thread */
} ArgThread;
 
static long total_primos = 0;                 /* acumulador global */
static pthread_mutex_t mutex_total = PTHREAD_MUTEX_INITIALIZER;
static _Atomic long proximo_numero = 2;        /* fronteira de distribuicao dinamica */
 
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
 
    for (;;) {
        /* Reivindica atomicamente o proximo bloco de TAM_BLOCO numeros. */
        long ini = atomic_fetch_add_explicit(&proximo_numero, TAM_BLOCO,
                                              memory_order_relaxed);
        if (ini > a->limite) break;
        long fim = ini + TAM_BLOCO - 1;
        if (fim > a->limite) fim = a->limite;
 
        for (long n = ini; n <= fim; n++) {
            if (eh_primo(n)) local++;
        }
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
 
    total_primos = 0;
    atomic_store(&proximo_numero, 2);
 
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);
 
    for (int t = 0; t < P; t++) {
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

