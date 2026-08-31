#include <stdio.h>
#include <unistd.h>   // fork() e sleep()
#include <stdlib.h>   // system()

int main() {

  int pid;

  // fork() cria um novo processo.
  //
  // Depois do fork(), existem DOIS processos executando
  // o programa a partir deste ponto:
  //
  //              fork()
  //                |
  //          +-----+-----+
  //          |           |
  //         PAI         FILHO
  //
  // O valor retornado pelo fork() é diferente para cada um:
  //
  // PAI   -> recebe o PID do filho (valor > 0)
  // FILHO -> recebe 0
  //
  // Em caso de erro, fork() retorna -1.
  pid = fork();


  // ============================
  // PROCESSO FILHO
  // ============================
  //
  // Se pid == 0, estamos no processo filho.
  if (pid == 0) {

    // system() executa um comando do sistema operacional.
    //
    // "ps -ef" mostra os processos atualmente em execução.
    //
    // Neste momento, o filho mostra a lista de processos.
    system("ps -ef");


    // O filho fica "dormindo" durante aproximadamente 10 segundos.
    //
    // IMPORTANTE:
    // sleep() NÃO garante que o pai vai executar primeiro.
    // Ele apenas suspende o processo que chamou sleep().
    sleep(10);


    // Depois de aproximadamente 10 segundos,
    // o filho executa "ps -ef" novamente.
    //
    // Assim podemos comparar a lista de processos
    // antes e depois dos 10 segundos.
    system("ps -ef");


  // ============================
  // PROCESSO PAI
  // ============================
  //
  // Se pid for diferente de 0, estamos no pai.
  //
  // Nesse caso, pid contém o PID do processo filho.
  } else {

    // O pai também é colocado para dormir,
    // mas por aproximadamente 3 segundos.
    sleep(3);
  }


  // Depois que o processo termina seu bloco de código,
  // return 0 indica que o programa terminou normalmente.
  //
  // Tanto o pai quanto o filho chegam aqui,
  // porque ambos continuam executando depois do fork().
  return 0;
}

