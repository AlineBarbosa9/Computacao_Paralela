#include <stdio.h>
#include <unistd.h>   // fork(), sleep()
#include <stdlib.h>   // system()
#include <sys/wait.h> // wait()

int main() {
  int pid;
  int status;

  // Cria um novo processo.
  // Pai: pid recebe o PID do filho (> 0)
  // Filho: pid recebe 0
  pid = fork();

  // Processo filho
  if (pid == 0) {

    // Mostra os processos em execução.
    system("ps -ef");

    // Filho dorme por 10 segundos.
    sleep(10);

    // Mostra novamente os processos.
    system("ps -ef");

    // Encerra o filho retornando status 3.
    return 3;

  } else {

    // Pai dorme por 3 segundos.
    sleep(3);

    // Pai espera o filho terminar e armazena
    // seu status de saída em "status".
    wait(&status);

    // Exibe o status em decimal e hexadecimal.
    printf("No processo pai: status de saída do filho é decimal %d, hexa %0x\n",
           status, status);
  }

  return 0;
}


