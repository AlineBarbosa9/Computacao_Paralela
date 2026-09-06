#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM_KEY 0x5678
#define BUF_SIZE 1024

struct shmseg {
    int complete;
    char palavra[BUF_SIZE];
};

int main() {
    int shmid;
    struct shmseg *shmp;
    int tamanho;
    int i;

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

    // Espera a palavra ser enviada
    while (shmp->complete != 1) {
        sleep(1);
    }

    printf("Palavra recebida: %s\n", shmp->palavra);

    // Obtém o tamanho da palavra
    tamanho = strlen(shmp->palavra);

    // Inverte a palavra
    printf("Palavra invertida: ");

    for (i = tamanho - 1; i >= 0; i--) {
        printf("%c", shmp->palavra[i]);
    }

    printf("\n");

    // Desconecta e remove a memória
    shmdt(shmp);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
