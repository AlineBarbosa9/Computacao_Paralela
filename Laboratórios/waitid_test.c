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
  siginfo_t siginfo;

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
// Esperando pelo 3o processo filho
   status = waitid(P_PID, pids[total_processos - 1], &siginfo, WEXITED);
   if (status == -1) {
      perror("Erro em waitid");
      return 1;
   }
   printf("Informações recebidas do waitid são: ");
   printf("PID do filho: %d,\n   ID de usuário real do filho: %d\n", siginfo.si_pid, siginfo.si_uid);
   return 0;  
  
}