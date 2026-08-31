#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char *argv[]) {

   // childpid vai armazenar o valor retornado pelo fork()
   // Pai: fork() retorna o PID do filho (valor > 0)
   // Filho: fork() retorna 0
   pid_t childpid = 0;

   int i, n;

   // O programa espera exatamente 1 argumento:
   // o número de processos que queremos criar.
   //
   // Exemplo:
   // ./programa 4
   //
   // Nesse caso:
   // argc = 2
   // argv[1] = "4"
   if (argc != 2) {
      fprintf(stderr, "Usage: %s processes\n", argv[0]);
      return 1;
   }

   // atoi() converte o argumento de texto para inteiro.
   //
   // Exemplo:
   // argv[1] = "4"
   // n = 4
   n = atoi(argv[1]);


   // O loop começa com i = 1 e continua enquanto i < n.
   //
   // Para n = 4:
   // i = 1
   // i = 2
   // i = 3
   //
   // A ideia é criar uma cadeia de processos.
   for (i = 1; i < n; i++)

      // fork() cria um novo processo.
      //
      // Depois do fork(), temos DOIS processos executando
      // o código a partir deste ponto:
      //
      //             fork()
      //               |
      //          +----+----+
      //          |         |
      //         PAI      FILHO
      //       > 0          0
      //
      // No PAI:
      //   childpid recebe o PID do filho.
      //   Como o valor é diferente de zero, o if é verdadeiro.
      //   Então o pai executa break e sai do loop.
      //
      // No FILHO:
      //   childpid recebe 0.
      //   O if é falso.
      //   Portanto, o filho NÃO executa break e continua o loop.
      if (childpid = fork())
         break;


   // Cada processo chega aqui e imprime suas informações.
   //
   // getpid()  -> PID do próprio processo
   // getppid() -> PID do processo pai
   // childpid  -> PID do filho criado ou 0
   //
   // Exemplo de saída:
   //
   // i:1 process ID:1000 parent ID:900 child ID:1001
   // i:2 process ID:1001 parent ID:1000 child ID:1002
   // i:3 process ID:1002 parent ID:1001 child ID:1003
   // i:4 process ID:1003 parent ID:1002 child ID:0
   //
   // Os PIDs acima são apenas exemplos.
   fprintf(stderr, "i:%d process ID:%ld parent ID:%ld child ID:%ld\n",
           i, (long)getpid(), (long)getppid(), (long)childpid);

   return 0;
}

