#!/usr/bin/env bash
# Uso: ./bench.sh <executavel> <repeticoes> "<lista de threads>" [args do programa...]
# Ex.: ./bench.sh ./analise_sections 5 "1 2 4 8"
# Saída (stdout), uma linha por execução, em CSV: programa,threads,execucao,tempo_s
# O tempo é lido da linha "Tempo de execução ..." impressa pelo próprio programa
# (omp_get_wtime). Uma execução de aquecimento (descartada) precede cada conjunto.
set -euo pipefail
prog="$1"; reps="$2"; threads_list="$3"; shift 3
nome=$(basename "$prog")
for t in $threads_list; do
  OMP_NUM_THREADS=$t "$prog" "$@" > /dev/null          # aquecimento (cache de arquivos/páginas)
  for r in $(seq 1 "$reps"); do
    tempo=$(OMP_NUM_THREADS=$t "$prog" "$@" | grep -E "^Tempo de execução" | grep -oE '[0-9]+\.[0-9]+')
    echo "$nome,$t,$r,$tempo"
  done
done
