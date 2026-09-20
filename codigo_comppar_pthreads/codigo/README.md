# CompPar — Laboratório Pthreads

Programação concorrente em memória compartilhada com POSIX Threads e análise do
GIL do CPython.

## Estrutura

```
src/     códigos-fonte em C
python/  scripts da Parte 3
bin/     binários gerados (make)
bench.sh bateria automática de medições
```

## Compilação

```bash
make            # compila tudo com -O2 -Wall -Wextra -std=c11 -pedantic -pthread
make clean
```

## Execução

### Parte 1 — Conta corrente
```bash
make parte1     # Fase 0, Fase 1 (-O2 e -O0, 5x cada) e Fase 2
```

> **Atenção (-O2 × -O0):** com `-O2` o gcc mantém `saldo` em registrador durante
> todo o laço e grava na memória uma única vez. O resultado fica errado, porém
> estável (a última gravação sobrescreve a outra thread). Para observar a
> flutuação clássica de *lost updates* é preciso a variante `-O0`, que gera
> `load`/`add`/`store` reais a cada iteração. Os dois casos devem ser
> reportados.

### Parte 2 — Menu (opções A, B e C)
```bash
./bin/primos          5000000    <num_threads>
./bin/monte_carlo_pi  100000000  <num_threads>
./bin/soma_vetores    50000000   <num_threads>
```

`soma_vetores` com N = 50.000.000 aloca cerca de 1,2 GB (3 vetores de doubles).

### Parte 3 — Python e o GIL
```bash
make parte3     # seq.py, threads.py e processos.py
```

### Medições completas
```bash
REPS=5 bash bench.sh
```
Imprime tempos médios, Speedup e Eficiência para 1, 2, 4 e 8 threads e grava as
saídas brutas em `resultados/`.
