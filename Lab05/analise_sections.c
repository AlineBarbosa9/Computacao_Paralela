#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h> // Para INT_MAX, INT_MIN
#include <math.h>   // Para sqrt

#define N (1024 * 1024 * 100) // 100 Milhões de inteiros

int main() {
    int *data = (int*) malloc(N * sizeof(int));
    if (!data) { perror("malloc falhou"); return 1; }

    long long i;

    // Inicializa com dados (exemplo)
    printf("Inicializando dados...\n");
    #pragma omp parallel for
    for (i = 0; i < N; i++) {
        data[i] = (i % 1000) - 500; // Valores entre -500 e 499
    }
    printf("Dados inicializados.\n");

    int min_val = INT_MAX;
    int max_val = INT_MIN;
    long long sum = 0;
    long long sum_sq = 0; // Soma dos quadrados

    double start_time, end_time;
    start_time = omp_get_wtime();

    // --- Bloco a ser otimizado ---
    printf("Calculando métricas (versão sections)...\n");

    // Uma única região paralela; cada métrica em uma section distinta.
    // As reduções ficam na diretiva combinada: cada thread recebe cópias
    // privadas (inicializadas com o elemento neutro) que são combinadas no fim.
    // O índice do laço é declarado DENTRO de cada section: um "long long i"
    // declarado fora seria compartilhado entre as threads (condição de corrida).
    #pragma omp parallel sections reduction(min:min_val) reduction(max:max_val) \
                                  reduction(+:sum, sum_sq)
    {
        #pragma omp section
        {
            for (long long k = 0; k < N; k++) {
                if (data[k] < min_val) min_val = data[k];
            }
        }

        #pragma omp section
        {
            for (long long k = 0; k < N; k++) {
                if (data[k] > max_val) max_val = data[k];
            }
        }

        #pragma omp section
        {
            for (long long k = 0; k < N; k++) {
                sum    += data[k];
                sum_sq += (long long)data[k] * data[k];
            }
        }
    }
    printf("Min, Max, Soma e Soma Quadrados calculados.\n");
    // --- Fim do Bloco ---

    end_time = omp_get_wtime();

    double average = (double)sum / N;
    double variance = ((double)sum_sq / N) - (average * average);
    double std_dev = sqrt(variance);

    printf("\n--- Resultados ---\n");
    printf("Min: %d\n", min_val);
    printf("Max: %d\n", max_val);
    printf("Soma: %lld\n", sum);
    printf("Média: %.2f\n", average);
    printf("Desvio Padrão: %.2f\n", std_dev);
    printf("Tempo de execução (cálculo): %.4f segundos\n", end_time - start_time);

    free(data);
    return 0;
}
