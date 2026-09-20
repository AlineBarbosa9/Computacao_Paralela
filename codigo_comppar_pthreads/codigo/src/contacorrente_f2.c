/* contacorrente_f2.c
 * Fase 2 - Correcao por exclusao mutua (pthread_mutex_t).
 *
 * A regiao critica e exatamente a instrucao que atualiza o saldo. O mutex
 * serializa o trio load-add-store, restaurando o determinismo do resultado
 * (150.001.000,00) ao custo de 100.000.000 de pares lock/unlock.
 *
 * Compilacao: gcc -O2 -Wall -Wextra contacorrente_f2.c -o contacorrente_f2 -pthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_OPERACOES 50000000
#define VALOR_DEPOSITO 5.0
#define VALOR_SAQUE 2.0

double saldo = 1000.00;

/* Mutex que protege o acesso a variavel `saldo`. */
pthread_mutex_t mutex_saldo = PTHREAD_MUTEX_INITIALIZER;

void *thread_depositos(void *arg) {
    (void)arg;
    for (long i = 0; i < NUM_OPERACOES; i++) {
        pthread_mutex_lock(&mutex_saldo);   /* entrada na regiao critica */
        saldo += VALOR_DEPOSITO;
        pthread_mutex_unlock(&mutex_saldo); /* saida da regiao critica  */
    }
    return NULL;
}

void *thread_saques(void *arg) {
    (void)arg;
    for (long i = 0; i < NUM_OPERACOES; i++) {
        pthread_mutex_lock(&mutex_saldo);
        saldo -= VALOR_SAQUE;
        pthread_mutex_unlock(&mutex_saldo);
    }
    return NULL;
}

int main(void) {
    pthread_t t_dep, t_saq;
    struct timespec inicio, fim;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    if (pthread_create(&t_dep, NULL, thread_depositos, NULL) != 0) {
        perror("pthread_create (depositos)");
        return EXIT_FAILURE;
    }
    if (pthread_create(&t_saq, NULL, thread_saques, NULL) != 0) {
        perror("pthread_create (saques)");
        return EXIT_FAILURE;
    }

    pthread_join(t_dep, NULL);
    pthread_join(t_saq, NULL);

    pthread_mutex_destroy(&mutex_saldo);

    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Fase 2 - Mutex] Saldo final: %.2f | Tempo: %.4f s\n", saldo, tempo);
    return 0;
}
