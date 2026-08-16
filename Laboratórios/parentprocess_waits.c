#include <stdio.h>

int main() {
  int pid;
  int status;
  pid = fork();

  // Processo filho
  if (pid == 0) {
    system("ps -ef");
    sleep(10);
    system("ps -ef");
    return 3;  // Status de saída é 3 do processo filho
  } else {
    sleep(3);
    wait(&status);
    printf("No processo pai: status de saída do filho é decimal %d, hexa %0x\n", status, status);
  }
  return 0;
}