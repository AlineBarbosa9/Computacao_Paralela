#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipe1[2];  // Pai → Filho
    int pipe2[2];  // Filho → Pai
    int pipe3[2];  // Filho → Pai

    int pid;
    char palavra[100];
    char invertida[100];
    char resultado[100];

    // Cria os três pipes
    pipe(pipe1);
    pipe(pipe2);
    pipe(pipe3);

    // Cria o processo filho
    pid = fork();

    // Processo filho
    if (pid == 0) {

        // Lê a palavra enviada pelo pai
        read(pipe1[0], palavra, sizeof(palavra));

        // Descobre o tamanho da palavra
        int tamanho = strlen(palavra);

        // Inverte a palavra
        for (int i = 0; i < tamanho; i++) {
            invertida[i] = palavra[tamanho - 1 - i];
        }

        invertida[tamanho] = '\0';

        // Envia a palavra invertida para o pai
        write(pipe2[1], invertida, sizeof(invertida));

        // Verifica se a palavra é palíndrome
        if (strcmp(palavra, invertida) == 0) {
            strcpy(resultado, "A palavra e palindromo");
        } else {
            strcpy(resultado, "A palavra nao e palindromo");
        }

        // Envia o resultado para o pai
        write(pipe3[1], resultado, sizeof(resultado));

        return 0;

    } else {

        // Processo pai

        // Recebe a palavra do usuário
        printf("Digite uma palavra: ");
        scanf("%99s", palavra);

        // Envia a palavra para o filho pelo pipe1
        write(pipe1[1], palavra, sizeof(palavra));

        // Lê a palavra invertida enviada pelo filho
        read(pipe2[0], invertida, sizeof(invertida));

        printf("Palavra invertida: %s\n", invertida);

        // Lê o resultado da verificação
        read(pipe3[0], resultado, sizeof(resultado));

        printf("%s\n", resultado);

        // Espera o filho terminar
        wait(NULL);
    }

    return 0;
}
