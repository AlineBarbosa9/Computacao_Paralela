#include <stdio.h>
#include <unistd.h>

int main() {

    // Antes do fork(), há um processo apenas
    printf("Antes do fork\n");

   fork();
   printf("Depois do fork\n");

   // Após o fork(), há o processo pai e o processo filho
   
   // Saída:
   // Antes do fork
   // Depois do fork
   // Depois do fork

   // Os dois continuam executando o programa a partir do ponto depois do fork()

    return 0;
}
