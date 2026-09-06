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

    // Cria/obtém a memória compartilhada
    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644 | IPC_CREAT);

    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Anexa a memória ao processo
    shmp = shmat(shmid, NULL, 0);

    if (shmp == (void *)-1) {
        perror("shmat");
        return 1;
    }

    // Lê a palavra do usuário
    printf("Digite uma palavra: ");
    scanf("%1023s", shmp->palavra);

    // Indica que a palavra está disponível
    shmp->complete = 1;

    printf("Palavra enviada: %s\n", shmp->palavra);

    // Desconecta da memória
    shmdt(shmp);

    return 0;
}
