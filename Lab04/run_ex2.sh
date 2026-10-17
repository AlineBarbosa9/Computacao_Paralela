#!/bin/bash
# Exercício 2: 10 execuções por (versão, nº de threads).
#  - Escalabilidade FORTE: N fixo = 2^28 (1 GiB de int32), threads 1..6
#  - Escalabilidade FRACA: N = threads * 2^27 (trabalho por thread constante;
#    máximo 6 * 2^27 = 805.306.368 <= 2^30), threads 1..6
# Dica: use REPS=2 N_FORTE=$((2**22)) BASE_FRACA=$((2**21)) ./run_ex2.sh para um teste rápido.
REPS=${REPS:-10}
N_FORTE=${N_FORTE:-$((2**28))}
BASE_FRACA=${BASE_FRACA:-$((2**27))}
mkdir -p results
for v in critical reduction; do
  : > results/ex2_${v}_forte.csv
  : > results/ex2_${v}_fraca.csv
  for t in 1 2 3 4 5 6; do
    ./ex2_soma_${v} $t $N_FORTE $REPS >> results/ex2_${v}_forte.csv
    ./ex2_soma_${v} $t $((t * BASE_FRACA)) $REPS >> results/ex2_${v}_fraca.csv
  done
done
echo "Resultados em results/ex2_*.csv"
