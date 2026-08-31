#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
  int pid;
  int pids[3];
  int status;
  int num_processos = 0;
  int total_processos = 3;

  while (num_processos < total_processos) {
    pid = fork();

    // Processo filho
    if (pid == 0) {
      printf("No processo filho: ID do processo é %d\n", getpid());
      sleep(5);
      return 4;
    } else {
      pids[num_processos] = pid;
      num_processos++;
      printf("No processo pai: processo número %d criado\n", pid);
    }
  }

  // Aguardando o 3º processo filho
  waitpid(pids[total_processos - 1], &status, 0);
  if (WIFEXITED(status) != 0) {
    printf("Processo %d encerrado normalmente\n", pids[total_processos - 1]);  
    printf("Status de saída do filho é %d\n", WEXITSTATUS(status));
  } else {
    printf("Processo %d não encerrado normalmente\n", pids[total_processos - 1]);
  }

  return 0;
}