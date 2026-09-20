/* contacorrente_f4_atomic.c
 * Extra (Questao 3-b) - Alternativa com operacoes atomicas nativas do C11.
 *
 * Observacao tecnica importante: <stdatomic.h> nao oferece atomic_fetch_add
 * para tipos de ponto flutuante. Por isso o saldo e mantido em CENTAVOS,
 * usando um inteiro atomico de 64 bits (_Atomic long long). A atualizacao
 * passa a ser uma unica instrucao "lock xadd" no x86-64, sem chamada de
 * biblioteca e sem possibilidade de bloqueio/troca de contexto.
 *
 * Compilacao: gcc -O2 -Wall -Wextra -std=c11 contacorrente_f4_atomic.c -o contacorrente_f4 -pthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define NUM_OPERACOES 50000000
#define VALOR_DEPOSITO_CENT 500LL /* R$ 5,00 */
#define VALOR_SAQUE_CENT    200LL /* R$ 2,00 */

/* Saldo em centavos: 1000,00 -> 100000 */
_Atomic long long saldo_centavos = 100000LL;

void *thread_depositos(void *arg) {
    (void)arg;
    for (long i = 0; i < NUM_OPERACOES; i++) {
        atomic_fetch_add_explicit(&saldo_centavos, VALOR_DEPOSITO_CENT,
                                  memory_order_relaxed);
    }
    return NULL;
}

void *thread_saques(void *arg) {
    (void)arg;
    for (long i = 0; i < NUM_OPERACOES; i++) {
        atomic_fetch_sub_explicit(&saldo_centavos, VALOR_SAQUE_CENT,
                                  memory_order_relaxed);
    }
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

    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    double saldo = atomic_load(&saldo_centavos) / 100.0;

    printf("[Extra - stdatomic] Saldo final: %.2f | Tempo: %.4f s\n",
           saldo, tempo);
    return 0;
}
