#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Variável Global
double saldo = 0;

// Função Para Depósito de 5 Unidades Monetárias
void depositos(){
    saldo += 5;
}

// Função Para Saque de 2 Unidades Monetárias
void saques(){
    saldo -= 2;
}

int main() {
    // Início de Medição de Tempo
    clock_t inicio = clock();

    saldo = 1000.00;

    // Realiza 214748300 Depósitos de 5 Unidades Monetárias
    for(int i = 0; i < 214748300; i++){
        depositos();
    }

    // Realiza 2147483000 Saques de 2 Unidades Monetárias
    for(int i = 0; i < 2147483000; i++){
        saques();
    }

    printf("Saldo final: %.2lf\n", saldo);

    // Fim de Medição de Tempo e Impressão na Tela
    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %.2f segundos\n", tempo);

    return 0;
}
