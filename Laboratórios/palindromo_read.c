#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define BUF_SIZE 1024

struct shmseg {
    int complete;
    char palavra[BUF_SIZE];
};

int main() {
    int shmid;
    struct shmseg *shmp;
    int inicio, fim;
    int palindromo = 1;

    // Obtém a memória compartilhada
    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644 | IPC_CREAT);

    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Anexa a memória
    shmp = shmat(shmid, NULL, 0);

    if (shmp == (void *)-1) {
        perror("shmat");
        return 1;
    }

    // Espera o escritor enviar a palavra
    while (shmp->complete != 1) {
        sleep(1);
    }

    printf("Palavra recebida: %s\n", shmp->palavra);

    // Verifica os caracteres das extremidades para o centro
    inicio = 0;
    fim = strlen(shmp->palavra) - 1;

    while (inicio < fim) {
        if (shmp->palavra[inicio] != shmp->palavra[fim]) {
            palindromo = 0;
            break;
        }

        inicio++;
        fim--;
    }

    if (palindromo)
        printf("A palavra e um PALINDROMO.\n");
    else
        printf("A palavra NAO e um PALINDROMO.\n");

    // Desconecta e remove a memória
    shmdt(shmp);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
