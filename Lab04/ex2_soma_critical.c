/*
 * Nome: Aline Barbosa Vidal RA: 10721348
 * CompPar - Lab Explorando-OpenMP - Exercício 2 (versão CRITICAL)
 * Soma de um vetor com variável compartilhada protegida por #pragma omp critical.
 *
 * Compilar: gcc -O2 -fopenmp ex2_soma_critical.c -o ex2_soma_critical
 * Executar: ./ex2_soma_critical <threads> <N> <repeticoes>
 * Saída (CSV, uma linha por repetição): versao,threads,N,rep,tempo_s
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <threads> <N> <repeticoes>\n", argv[0]);
        return 1;
    }
    int threads = atoi(argv[1]);
    size_t n = strtoull(argv[2], NULL, 10);
    int reps = atoi(argv[3]);

    int32_t *v = malloc(n * sizeof(int32_t));
    if (!v) { fprintf(stderr, "Sem memoria para N=%zu\n", n); return 1; }

    /* Dados determinísticos: v[i] = (i % 1000) + 1 -> mesmo resultado sempre.
     * Inicialização paralela (first-touch) para distribuir páginas de memória. */
#pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < n; i++)
        v[i] = (int32_t)(i % 1000) + 1;

    /* Resultado esperado em forma fechada (cada bloco de 1000 soma 500500) */
    long long esperado = (long long)(n / 1000) * 500500LL;
    long long r = (long long)(n % 1000);
    esperado += r * (r + 1) / 2;

    for (int rep = 1; rep <= reps; rep++) {
        long long soma = 0; /* variável compartilhada */
        double t0 = omp_get_wtime();
#pragma omp parallel num_threads(threads)
        {
            long long parcial = 0;
#pragma omp for
            for (size_t i = 0; i < n; i++)
                parcial += v[i];
#pragma omp critical
            soma += parcial; /* seção crítica: 1 acesso por thread */
        }
        double t1 = omp_get_wtime();
        if (soma != esperado) {
            fprintf(stderr, "ERRO: soma=%lld esperado=%lld\n", soma, esperado);
            return 2;
        }
        printf("critical,%d,%zu,%d,%.6f\n", threads, n, rep, t1 - t0);
        if (rep == reps)
            fprintf(stderr, "[critical] threads=%d N=%zu soma=%lld (OK)\n", threads, n, soma);
    }
    free(v);
    return 0;
}
