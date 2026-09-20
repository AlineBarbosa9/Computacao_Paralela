/* contacorrente_f3_reducao.c
 * Extra (Questao 3-a) - Alternativa sem 100 milhoes de locks: cada thread
 * acumula em uma variavel LOCAL (privada, mantida em registrador/pilha) e,
 * ao final, realiza uma unica reducao protegida por mutex.
 *
 * Resultado: 2 operacoes de lock no programa inteiro, em vez de 10^8.
 *
 * Compilacao: gcc -O2 -Wall -Wextra contacorrente_f3_reducao.c -o contacorrente_f3 -pthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_OPERACOES 50000000
#define VALOR_DEPOSITO 5.0
#define VALOR_SAQUE 2.0

double saldo = 1000.00;
pthread_mutex_t mutex_saldo = PTHREAD_MUTEX_INITIALIZER;

void *thread_depositos(void *arg) {
    (void)arg;
    double local = 0.0; /* acumulador privado da thread */
    for (long i = 0; i < NUM_OPERACOES; i++) {
        local += VALOR_DEPOSITO;
    }
    pthread_mutex_lock(&mutex_saldo);   /* unica secao critica */
    saldo += local;
    pthread_mutex_unlock(&mutex_saldo);
    return NULL;
}

void *thread_saques(void *arg) {
    (void)arg;
    double local = 0.0;
    for (long i = 0; i < NUM_OPERACOES; i++) {
        local += VALOR_SAQUE;
    }
    pthread_mutex_lock(&mutex_saldo);
    saldo -= local;
    pthread_mutex_unlock(&mutex_saldo);
    return NULL;
}

int main(void) {
    pthread_t t_dep, t_saq;
    struct timespec inicio, fim;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    pthread_create(&t_dep, NULL, thread_depositos, NULL);
    pthread_create(&t_saq, NULL, thread_saques, NULL);
    pthread_join(t_dep, NULL);
    pthread_join(t_saq, NULL);

    pthread_mutex_destroy(&mutex_saldo);
    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Extra - Reducao local] Saldo final: %.2f | Tempo: %.4f s\n",
           saldo, tempo);
    return 0;
}
