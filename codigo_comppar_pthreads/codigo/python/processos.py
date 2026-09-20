"""processos.py - Extra (Questao 8): mesma carga com multiprocessing.

Cada processo possui seu proprio interpretador e seu proprio GIL, portanto ha
paralelismo real. O custo e a criacao do processo e a serializacao (pickle)
dos argumentos e resultados.
"""
import time
import multiprocessing


def contagem_pesada(n):
    while n > 0:
        n -= 1


if __name__ == "__main__":
    n = 100_000_000
    inicio = time.time()

    p1 = multiprocessing.Process(target=contagem_pesada, args=(n,))
    p2 = multiprocessing.Process(target=contagem_pesada, args=(n,))
    p1.start()
    p2.start()
    p1.join()
    p2.join()

    fim = time.time()
    print(f"[Python com Processos] Tempo: {fim - inicio:.4f} segundos")
