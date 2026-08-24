#include <stdio.h>
#include <unistd.h>

int main() {

    pid_t pid = fork();
    // pid == 0 -> filho
    // pid > 0 -> pai
    // pid < 0 -> erro

    if (pid == 0) {
        printf("Filho: meu PID é %d\n", getpid());
        printf("Filho: PID do meu pai é %d\n", getppid());

    } else {
        printf("Pai: meu PID é %d\n", getpid());
        printf("Pai: PID do meu filho é %d\n", pid);
    }

    // pai pode terminar o processo antes do filho em algumas execuções
    // filho fica orfâo -> é adotado por init e retorna o pid do novo pai (1)
    // init é o primeiro processo iniciado pelo sistema operacional
    // são processos independentes, o SO decide qual alocar primeiro
    
    return 0;
}