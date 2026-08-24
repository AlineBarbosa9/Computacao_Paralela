#include <stdio.h>
#include <unistd.h>

int main() {

    printf("Antes do fork\n");

    fork();

    printf("Depois do fork\n");

    return 0;
}
