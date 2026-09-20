# Makefile - CompPar: Laboratorio Pthreads
# Uso:
#   make            -> compila todos os binarios
#   make clean      -> remove binarios e resultados
#   make parte1     -> executa Fase 0, Fase 1 (5x) e Fase 2
#   make parte3     -> executa os scripts Python
#   make bench      -> executa a bateria completa de medicoes (bench.sh)

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -pedantic
LDFLAGS = -pthread
SRC     = src
BIN     = bin

ALVOS = $(BIN)/contacorrente_seq $(BIN)/contacorrente_f1 $(BIN)/contacorrente_f1_O0 \
        $(BIN)/contacorrente_f2 $(BIN)/contacorrente_f3 $(BIN)/contacorrente_f4 \
        $(BIN)/primos $(BIN)/monte_carlo_pi $(BIN)/soma_vetores

all: $(BIN) $(ALVOS)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/contacorrente_seq: $(SRC)/contacorrente_seq.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BIN)/contacorrente_f1: $(SRC)/contacorrente_f1.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Variante SEM otimizacao: em -O2 o gcc mantem `saldo` em registrador durante
# todo o laco e grava uma unica vez, mascarando a flutuacao do saldo. Com -O0
# cada iteracao gera load/add/store reais e a perda de atualizacoes aparece.
$(BIN)/contacorrente_f1_O0: $(SRC)/contacorrente_f1.c | $(BIN)
	$(CC) -O0 -Wall -Wextra -std=c11 $< -o $@ $(LDFLAGS)

$(BIN)/contacorrente_f2: $(SRC)/contacorrente_f2.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BIN)/contacorrente_f3: $(SRC)/contacorrente_f3_reducao.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BIN)/contacorrente_f4: $(SRC)/contacorrente_f4_atomic.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BIN)/primos: $(SRC)/primos.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BIN)/monte_carlo_pi: $(SRC)/monte_carlo_pi.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BIN)/soma_vetores: $(SRC)/soma_vetores.c | $(BIN)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

parte1: all
	@echo "=== Fase 0: Sequencial ==="; ./$(BIN)/contacorrente_seq
	@echo "=== Fase 1: Condicao de corrida - compilado com -O2 (5 execucoes) ==="
	@for i in 1 2 3 4 5; do ./$(BIN)/contacorrente_f1; done
	@echo "=== Fase 1: Condicao de corrida - compilado com -O0 (5 execucoes) ==="
	@for i in 1 2 3 4 5; do ./$(BIN)/contacorrente_f1_O0; done
	@echo "=== Fase 2: Mutex ==="; ./$(BIN)/contacorrente_f2

parte3:
	@echo "=== Python sequencial ==="; python3 python/seq.py
	@echo "=== Python threads ===";    python3 python/threads.py
	@echo "=== Python processos ===";  python3 python/processos.py

bench: all
	bash bench.sh

clean:
	rm -rf $(BIN) resultados

.PHONY: all clean parte1 parte3 bench
