#!/usr/bin/env bash
# bench.sh - Bateria automatica de medicoes do laboratorio.
# Executa cada configuracao REPS vezes, calcula a media aritmetica e ja
# imprime Speedup e Eficiencia prontos para serem colados no relatorio.
#
# Uso: bash bench.sh            (parametros padrao)
#      REPS=5 K=5000000 N_MC=100000000 N_VEC=50000000 bash bench.sh

set -u
REPS=${REPS:-5}
K=${K:-5000000}            # Opcao A: primos ate K
N_MC=${N_MC:-100000000}    # Opcao B: pontos de Monte Carlo
N_VEC=${N_VEC:-50000000}   # Opcao C: tamanho dos vetores
BIN=./bin
OUT=resultados
mkdir -p "$OUT"

extrai_tempo() { grep -oP 'Tempo:\s*\K[0-9.]+' ; }

media() { awk '{s+=$1; n++} END {if(n>0) printf "%.4f", s/n; else print "NA"}'; }

echo "Repeticoes por medicao: $REPS"
echo "Data: $(date -Is)"
echo

# ---------- PARTE 1 ----------
echo "=============== PARTE 1: CONTA CORRENTE ==============="
for prog in contacorrente_seq contacorrente_f1 contacorrente_f2 contacorrente_f3 contacorrente_f4; do
  [ -x "$BIN/$prog" ] || continue
  echo "--- $prog ---"
  : > "$OUT/$prog.txt"
  for i in $(seq 1 "$REPS"); do
    saida=$($BIN/$prog)
    echo "$saida" | tee -a "$OUT/$prog.txt"
  done
  m=$(grep -oP 'Tempo:\s*\K[0-9.]+' "$OUT/$prog.txt" | media)
  echo ">>> Tempo medio $prog: $m s"
  echo
done

# ---------- PARTE 2 ----------
roda_escalabilidade() {
  local nome="$1"; local prog="$2"; local param="$3"
  echo "=============== PARTE 2: $nome ==============="
  local t1=""
  printf "%-8s %-12s %-10s %-10s\n" "Threads" "Tempo(s)" "Speedup" "Eficiencia"
  for p in 1 2 4 8; do
    : > "$OUT/${prog}_${p}.txt"
    for i in $(seq 1 "$REPS"); do
      $BIN/$prog "$param" "$p" >> "$OUT/${prog}_${p}.txt"
    done
    local m
    m=$(grep -oP 'Tempo:\s*\K[0-9.]+' "$OUT/${prog}_${p}.txt" | media)
    if [ "$p" = "1" ]; then t1=$m; fi
    awk -v p="$p" -v m="$m" -v t1="$t1" 'BEGIN{
      s = (m>0) ? t1/m : 0;
      printf "%-8s %-12s %-10.3f %-10.3f\n", p, m, s, s/p;
    }'
  done
  echo
}

[ -x "$BIN/primos" ]         && roda_escalabilidade "PRIMOS (K=$K)"            primos         "$K"
[ -x "$BIN/monte_carlo_pi" ] && roda_escalabilidade "MONTE CARLO (N=$N_MC)"    monte_carlo_pi "$N_MC"
[ -x "$BIN/soma_vetores" ]   && roda_escalabilidade "SOMA VETORES (N=$N_VEC)"  soma_vetores   "$N_VEC"

# ---------- PARTE 3 ----------
echo "=============== PARTE 3: PYTHON / GIL ==============="
for script in seq threads processos; do
  [ -f "python/$script.py" ] || continue
  : > "$OUT/py_$script.txt"
  for i in $(seq 1 "$REPS"); do
    python3 "python/$script.py" | tee -a "$OUT/py_$script.txt"
  done
  m=$(grep -oP 'Tempo:\s*\K[0-9.]+' "$OUT/py_$script.txt" | media)
  echo ">>> Tempo medio $script.py: $m s"
  echo
done

echo "Saidas brutas gravadas em $OUT/"
