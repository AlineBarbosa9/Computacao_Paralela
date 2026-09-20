/* contacorrente_seq.c
 * Fase 0 - Baseline sequencial.
 * Ambas as operacoes sao executadas em serie na thread principal, de modo que
 * nao existe compartilhamento concorrente da variavel global `saldo`.
 * Serve como referencia de tempo (T_seq) e de corretude do resultado.
 *
 * Compilacao: gcc -O2 -Wall -Wextra contacorrente_seq.c -o contacorrente_seq
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_OPERACOES 50000000
#define VALOR_DEPOSITO 5.0
#define VALOR_SAQUE 2.0

/* Variavel global: representa o saldo da conta corrente. */
double saldo = 1000.00;

/* Realiza NUM_OPERACOES depositos sobre o saldo. */
void depositos(void) {
    for (long i = 0; i < NUM_OPERACOES; i++) {
        saldo += VALOR_DEPOSITO;
    }
}

/* Realiza NUM_OPERACOES saques sobre o saldo. */
void saques(void) {
    for (long i = 0; i < NUM_OPERACOES; i++) {
        saldo -= VALOR_SAQUE;
    }
}

int main(void) {
    struct timespec inicio, fim;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    depositos();
    saques();

    clock_gettime(CLOCK_MONOTONIC, &fim);

    double tempo = (fim.tv_sec - inicio.tv_sec) +
                   (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Sequencial] Saldo final: %.2f | Tempo: %.4f s\n", saldo, tempo);
    return 0;
}
