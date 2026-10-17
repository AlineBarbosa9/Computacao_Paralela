#!/bin/bash
# Exercício 3: média de 3 execuções. N=2048 por padrão (3 matrizes = 96 MB,
# bem maior que L2 -> o uso de cache importa). Use N=4096 se o tempo for curto.
N=${N:-2048}
REPS=${REPS:-3}
mkdir -p results
./ex3_matmul seq 1 $N $REPS > results/ex3_seq.csv
: > results/ex3_omp.csv
for t in 1 2 3 4 5 6; do
  ./ex3_matmul omp $t $N $REPS >> results/ex3_omp.csv
done
echo "Resultados em results/ex3_*.csv"
