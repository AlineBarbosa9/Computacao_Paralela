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
    printf("Simulando um passo (versão inicial)...\n");
    const double GRAVITY = -9.81;

    // 1. Atualiza Posições
    #pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x += particles[i].vx * DT;
        particles[i].y += particles[i].vy * DT;
        particles[i].z += particles[i].vz * DT;
    }
    printf("  Posições atualizadas.\n");

    // 2. Atualiza Velocidades (baseado em força externa simples)
    #pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++) {
        // Exemplo: Gravidade agindo em Y
        particles[i].vy += GRAVITY * DT;
    }
    printf("  Velocidades atualizadas.\n");
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
