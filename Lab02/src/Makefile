# Makefile - Laboratorio CompPar: Hierarquia de Memoria, Cache e Pthreads
CC      = gcc
CFLAGS_O0 = -O0 -Wall -g
CFLAGS_O3 = -O3 -Wall
LDFLAGS_PT = -pthread

SEQ_SRCS = varredura_linha varredura_coluna matmul_padrao matmul_bloco
PT_SRCS  = matmul_pthreads matmul_pthreads_bloco

O0_BINS = $(addsuffix _O0,$(SEQ_SRCS)) $(addsuffix _O0,$(PT_SRCS))
O3_BINS = $(addsuffix _O3,$(SEQ_SRCS)) $(addsuffix _O3,$(PT_SRCS))

all: $(O0_BINS) $(O3_BINS)

# --- versoes sequenciais ---
%_O0: %.c
	$(CC) $(CFLAGS_O0) $< -o $@

%_O3: %.c
	$(CC) $(CFLAGS_O3) $< -o $@

# --- versoes com pthreads (precisam de -pthread) ---
matmul_pthreads_O0: matmul_pthreads.c
	$(CC) $(CFLAGS_O0) $< -o $@ $(LDFLAGS_PT)

matmul_pthreads_O3: matmul_pthreads.c
	$(CC) $(CFLAGS_O3) $< -o $@ $(LDFLAGS_PT)

matmul_pthreads_bloco_O0: matmul_pthreads_bloco.c
	$(CC) $(CFLAGS_O0) $< -o $@ $(LDFLAGS_PT)

matmul_pthreads_bloco_O3: matmul_pthreads_bloco.c
	$(CC) $(CFLAGS_O3) $< -o $@ $(LDFLAGS_PT)

clean:
	rm -f $(O0_BINS) $(O3_BINS)

.PHONY: all clean
