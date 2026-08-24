#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    pid_t pid = fork();

    if (pid == 0) {

        printf("Filho: estou executando!\n");

    } else {

        wait(NULL);
        // NULL significa que não queremos guardar informações sobre
        // como o processo filho terminou.
        // O wait() ainda funciona normalmente: ele apenas espera
        // o processo filho terminar antes de o pai continuar.

        printf("Pai: o filho terminou!\n");
    }

    return 0;
}