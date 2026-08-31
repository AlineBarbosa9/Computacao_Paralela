#include <stdio.h>
#include <unistd.h>

int main() {
   int pipefds[2];       // [0] leitura, [1] escrita
   int returnstatus;
   int pid;
   char writemessages[2][20] = {"Hi", "Hello"};
   char readmessage[20]; // Buffer para leitura

   // Cria o pipe
   returnstatus = pipe(pipefds);

   if (returnstatus == -1) {
      printf("Unable to create pipe\n");
      return 1;
   }

   // Cria o processo filho
   pid = fork();

   // Processo filho
   if (pid == 0) {

      // Lê a primeira mensagem
      read(pipefds[0], readmessage, sizeof(readmessage));
      printf("Child Process - Reading from pipe – Message 1 is %s\n",
             readmessage);

      // Lê a segunda mensagem
      read(pipefds[0], readmessage, sizeof(readmessage));
      printf("Child Process - Reading from pipe – Message 2 is %s\n",
             readmessage);

   } else { // Processo pai

      // Envia a primeira mensagem
      printf("Parent Process - Writing to pipe - Message 1 is %s\n",
             writemessages[0]);
      write(pipefds[1], writemessages[0], sizeof(writemessages[0]));

      // Envia a segunda mensagem
      printf("Parent Process - Writing to pipe - Message 2 is %s\n",
             writemessages[1]);
      write(pipefds[1], writemessages[1], sizeof(writemessages[1]));
   }

   return 0;
}
