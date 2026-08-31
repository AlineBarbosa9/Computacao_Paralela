#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

   pid_t childpid;
   int i, n;

   // O programa espera exatamente 1 argumento:
   // a quantidade de filhos que queremos criar.
   //
   // Exemplo:
   // ./programa 4
   if (argc != 2) {
      fprintf(stderr, "Usage: %s processes\n", argv[0]);
      return 1;
   }

   n = atoi(argv[1]);

   // O pai percorre TODO o loop.
   //
   // Para n = 4:
   // i = 1 -> cria Filho 1
   // i = 2 -> cria Filho 2
   // i = 3 -> cria Filho 3
   //
   // O pai NÃO executa break.
   for (i = 1; i < n; i++) {

      childpid = fork();

      // Se fork() retornar 0, estamos no FILHO.
      //
      // O filho não deve continuar o loop, porque queremos
      // que SOMENTE o processo original crie os filhos.
      if (childpid == 0) {
         break;
      }

      // Se chegamos aqui, estamos no PAI.
      //
      // O pai continua o loop e cria o próximo filho.
   }

   // Cada processo imprime suas informações.
   //
   // O pai terá childpid = PID do último filho criado.
   // Cada filho terá childpid = 0.
   fprintf(stderr,
           "i:%d process ID:%ld parent ID:%ld child ID:%ld\n",
           i,
           (long)getpid(),
           (long)getppid(),
           (long)childpid);

   return 0;
}
