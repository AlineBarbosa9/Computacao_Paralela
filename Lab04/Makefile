CC     = gcc
CFLAGS = -O2 -fopenmp -Wall

all: ex1_trapezio ex2_soma_critical ex2_soma_reduction ex3_matmul

ex1_trapezio: ex1_trapezio.c
	$(CC) $(CFLAGS) $< -o $@ -lm
%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f ex1_trapezio ex2_soma_critical ex2_soma_reduction ex3_matmul
