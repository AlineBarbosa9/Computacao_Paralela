/*
 * Nome: <SEU NOME AQUI>
 * CompPar - Lab Explorando-OpenMP - Exercício 1
 * Integral pela regra do trapézio com OpenMP.
 *
 * Cada versão corresponde a uma etapa dos slides (páginas 24 a 38):
 *   1 -> slides 24-25: Trap() com local_a/local_b/local_n e critical no final
 *   2 -> slide 31    : critical envolvendo a chamada de Local_trap() (vira serial!)
 *   3 -> slide 32    : variável privada + critical depois da chamada
 *   4 -> slides 33-35: cláusula reduction(+: global_result) no parallel
 *   5 -> slide 38    : parallel for + reduction (laço do trapézio direto)
 *
 * Compilar: gcc -O2 -fopenmp ex1_trapezio.c -o ex1_trapezio -lm
 * Executar: ./ex1_trapezio <threads> <versao 1-5> <a> <b> <n>
 *   ex.:    ./ex1_trapezio 4 5 0 3 100000000      (f(x)=x*x, resultado exato = 9)
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static double f(double x) { return x * x; }

/* Parte "local" de cada thread: o intervalo [a,b] é dividido em blocos
 * consecutivos de n/thread_count trapézios (exige n % thread_count == 0). */
static double Local_trap(double a, double b, int n) {
    double h = (b - a) / n;
    int my_rank = omp_get_thread_num();
    int thread_count = omp_get_num_threads();
    int local_n = n / thread_count;
    double local_a = a + my_rank * local_n * h;
    double local_b = local_a + local_n * h;
    double my_result = (f(local_a) + f(local_b)) / 2.0;
    for (int i = 1; i <= local_n - 1; i++)
        my_result += f(local_a + i * h);
    return my_result * h;
}

/* Versão 1 (slides 24-25) */
static void Trap(double a, double b, int n, double *global_result_p) {
    double my_result = Local_trap(a, b, n);
#pragma omp critical
    *global_result_p += my_result;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s <threads> <versao 1-5> <a> <b> <n>\n", argv[0]);
        return 1;
    }
    int thread_count = (int)strtol(argv[1], NULL, 10);
    int versao = (int)strtol(argv[2], NULL, 10);
    double a = strtod(argv[3], NULL), b = strtod(argv[4], NULL);
    int n = (int)strtol(argv[5], NULL, 10);
    double global_result = 0.0;

    if (versao <= 4 && n % thread_count != 0) {
        fprintf(stderr, "Nas versoes 1-4 n deve ser multiplo de threads.\n");
        return 1;
    }

    double t0 = omp_get_wtime();
    switch (versao) {
    case 1:
#pragma omp parallel num_threads(thread_count)
        Trap(a, b, n, &global_result);
        break;
    case 2:
#pragma omp parallel num_threads(thread_count)
        {
            /* critical englobando a chamada: threads executam uma por vez */
#pragma omp critical
            global_result += Local_trap(a, b, n);
        }
        break;
    case 3:
#pragma omp parallel num_threads(thread_count)
        {
            double my_result = 0.0;           /* privada (declarada no bloco) */
            my_result += Local_trap(a, b, n); /* trabalho em paralelo */
#pragma omp critical
            global_result += my_result;       /* só a soma é serializada */
        }
        break;
    case 4:
#pragma omp parallel num_threads(thread_count) reduction(+ : global_result)
        global_result += Local_trap(a, b, n);
        break;
    case 5: {
        double h = (b - a) / n;
        double approx = (f(a) + f(b)) / 2.0;
#pragma omp parallel for num_threads(thread_count) reduction(+ : approx)
        for (int i = 1; i <= n - 1; i++)
            approx += f(a + i * h);
        global_result = h * approx;
        break;
    }
    default:
        fprintf(stderr, "Versao invalida.\n");
        return 1;
    }
    double t1 = omp_get_wtime();

    printf("Versao %d | threads = %d | n = %d trapezios\n", versao, thread_count, n);
    printf("Estimativa da integral de %f a %f = %.14e\n", a, b, global_result);
    printf("Tempo: %.6f s\n", t1 - t0);
    return 0;
}
