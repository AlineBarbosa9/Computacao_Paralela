#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

#define NUM_PARTICLES (1024 * 1024 * 10)
#define DT 0.01 // Time step

typedef struct {
    double x, y, z;    // Posição
    double vx, vy, vz; // Velocidade
} Particle;

// Gerador pseudoaleatório determinístico e thread-safe (splitmix64).
// Substitui rand(), que não é thread-safe e torna o resultado dependente
// da ordem de execução das threads. Retorna um valor em [0, 1).
static inline double rand01(uint64_t idx, uint64_t campo) {
    uint64_t z = idx * 6 + campo + 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    z = z ^ (z >> 31);
    return (double)(z >> 11) / 9007199254740992.0; // 2^53
}

int main() {
    Particle *particles = (Particle*) malloc(NUM_PARTICLES * sizeof(Particle));
    if (!particles) { perror("malloc falhou"); return 1; }

    long long i;

    // Inicializa partículas
    printf("Inicializando partículas...\n");
    #pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x  = rand01(i, 0) * 100.0;
        particles[i].y  = rand01(i, 1) * 100.0;
        particles[i].z  = rand01(i, 2) * 100.0;
        particles[i].vx = rand01(i, 3) * 10.0 - 5.0;
        particles[i].vy = rand01(i, 4) * 10.0 - 5.0;
        particles[i].vz = rand01(i, 5) * 10.0 - 5.0;
    }
    printf("Partículas inicializadas.\n");

    double start_time, end_time;
    start_time = omp_get_wtime();

    // --- Bloco a ser otimizado ---
    printf("Simulando um passo (versão sections)...\n");
    const double GRAVITY = -9.81;

#ifndef INGENUO
    // Versão CORRETA. A seção de posição LÊ vy e a de velocidade ESCREVE vy;
    // executadas ao mesmo tempo, y poderia usar a velocidade nova (condição de
    // corrida). Para preservar a semântica da versão sequencial (posição usa a
    // velocidade ANTIGA), a seção de velocidade grava em um buffer separado
    // (vy_novo) e o resultado é copiado de volta após o fim das seções.
    double *vy_novo = (double*) malloc(NUM_PARTICLES * sizeof(double));
    if (!vy_novo) { perror("malloc falhou"); return 1; }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (long long k = 0; k < NUM_PARTICLES; k++) {
                particles[k].x += particles[k].vx * DT;
                particles[k].y += particles[k].vy * DT;
                particles[k].z += particles[k].vz * DT;
            }
        }

        #pragma omp section
        {
            for (long long k = 0; k < NUM_PARTICLES; k++) {
                vy_novo[k] = particles[k].vy + GRAVITY * DT;
            }
        }
    } // barreira implícita: as duas seções terminaram

    // Publica as novas velocidades (custo extra da correção)
    #pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++) {
        particles[i].vy = vy_novo[i];
    }
    free(vy_novo);
#else
    // Versão INGÊNUA (apenas para demonstrar o problema): as duas seções
    // acessam particles[k].vy sem nenhuma ordenação -> condição de corrida.
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (long long k = 0; k < NUM_PARTICLES; k++) {
                particles[k].x += particles[k].vx * DT;
                particles[k].y += particles[k].vy * DT;
                particles[k].z += particles[k].vz * DT;
            }
        }

        #pragma omp section
        {
            for (long long k = 0; k < NUM_PARTICLES; k++) {
                particles[k].vy += GRAVITY * DT;
            }
        }
    }
#endif
    printf("  Posições e velocidades atualizadas.\n");
    // --- Fim do Bloco ---

    end_time = omp_get_wtime();
    printf("\nSimulação concluída.\n");

    // Checksum sequencial (fora da medição) para comparar versões
    double chk_y = 0.0, chk_vy = 0.0;
    for (i = 0; i < NUM_PARTICLES; i++) {
        chk_y  += particles[i].y;
        chk_vy += particles[i].vy;
    }
    printf("Checksum y: %.6f | Checksum vy: %.6f\n", chk_y, chk_vy);
    printf("Tempo de execução (simulação): %.4f segundos\n", end_time - start_time);

    free(particles);
    return 0;
}
