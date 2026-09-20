/* contacorrente_f1.c
 * Fase 1 - Concorrencia SEM sincronizacao (condicao de corrida proposital).
 *
 * Duas threads manipulam a mesma variavel global `saldo`. A operacao
 * `saldo += VALOR` nao e atomica: ela se decompoe em load -> add -> store.
 * Quando duas threads intercalam essas etapas, atualizacoes sao perdidas
 * (lost updates) e o saldo final passa a ser nao-deterministico.
 *
 * Compilacao: gcc -O2 -Wall -Wextra contacorrente_f1.c -o contacorrente_f1 -pthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_OPERACOES 50000000
#define VALOR_DEPOSITO 5.0
#define VALOR_SAQUE 2.0

/* Regiao compartilhada, acessada sem qualquer protecao. */
double saldo = 1000.00;

void *thread_depositos(void *arg) {
    (void)arg; /* argumento nao utilizado */
    for (long i = 0; i < NUM_OPERACOES; i++) {
        saldo += VALOR_DEPOSITO; /* RMW nao atomico */
    }
    return NULL;
}

void *thread_saques(void *arg) {
    (void)arg;
    for (long i = 0; i < NUM_OPERACOES; i++) {
        saldo -= VALOR_SAQUE; /* RMW nao atomico */
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

    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Fase 1 - Race Condition] Saldo final: %.2f | Tempo: %.4f s\n",
           saldo, tempo);
    return 0;
}
