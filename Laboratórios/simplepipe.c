#include <stdio.h>
#include <unistd.h>   // pipe(), write(), read()

int main() {

   // pipefds[0] = extremidade de leitura
   // pipefds[1] = extremidade de escrita
   int pipefds[2];

   int returnstatus;

   // Mensagens que serão enviadas pelo pipe.
   char writemessages[2][20] = {"Hi", "Hello"};

   // Buffer onde as mensagens serão recebidas.
   char readmessage[20];

   // Cria o pipe.
   returnstatus = pipe(pipefds);

   // Verifica se houve erro na criação.
   if (returnstatus == -1) {
      printf("Unable to create pipe\n");
      return 1;
   }

   // Envia a primeira mensagem pelo pipe.
   printf("Writing to pipe - Message 1 is %s\n", writemessages[0]);

   write(pipefds[1],
         writemessages[0],
         sizeof(writemessages[0]));

   // Lê a mensagem que está no pipe.
   read(pipefds[0],
        readmessage,
        sizeof(readmessage));

   printf("Reading from pipe – Message 1 is %s\n", readmessage);


   // Envia a segunda mensagem pelo pipe.
   printf("Writing to pipe - Message 2 is %s\n", writemessages[1]);

   write(pipefds[1],
         writemessages[1],
         sizeof(writemessages[0]));

   // Lê a segunda mensagem.
   read(pipefds[0],
        readmessage,
        sizeof(readmessage));

   printf("Reading from pipe – Message 2 is %s\n", readmessage);

   return 0;
}
