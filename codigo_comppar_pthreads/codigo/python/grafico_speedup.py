"""grafico_speedup.py - Gera os graficos de Speedup x Numero de Threads.

Preencha o dicionario TEMPOS com os tempos MEDIOS (em segundos) medidos na sua
maquina para 1, 2, 4 e 8 threads e execute:

    python3 grafico_speedup.py

Saida: speedup.png e eficiencia.png (prontos para inserir no relatorio).

Dependencia: matplotlib (pip install matplotlib)
"""
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

THREADS = [1, 2, 4, 8]

# >>> PREENCHER com os tempos medios medidos (segundos) <<<
TEMPOS = {
    "Primos (K=5.000.000)":        [None, None, None, None],
    "Monte Carlo (N=10^8)":        [None, None, None, None],
    "Soma de vetores (N=5x10^7)":  [None, None, None, None],
}

MARCADORES = ["o", "s", "^", "d"]


def calcula(tempos):
    """Speedup relativo a 1 thread e eficiencia."""
    t1 = tempos[0]
    speedup = [t1 / t for t in tempos]
    eficiencia = [s / p for s, p in zip(speedup, THREADS)]
    return speedup, eficiencia


def main():
    series = {k: v for k, v in TEMPOS.items() if all(x for x in v)}
    if not series:
        raise SystemExit("Preencha o dicionario TEMPOS antes de executar.")

    # ---- Grafico de Speedup ----
    fig, ax = plt.subplots(figsize=(7, 5))
    ax.plot(THREADS, THREADS, "k--", linewidth=1, label="Speedup ideal (linear)")
    for (nome, tempos), m in zip(series.items(), MARCADORES):
        s, _ = calcula(tempos)
        ax.plot(THREADS, s, marker=m, linewidth=1.8, label=nome)
    ax.set_xlabel("Numero de threads (p)")
    ax.set_ylabel("Speedup  $S_p = T_1 / T_p$")
    ax.set_title("Speedup x Numero de Threads")
    ax.set_xticks(THREADS)
    ax.grid(True, linestyle=":", alpha=0.6)
    ax.legend()
    fig.tight_layout()
    fig.savefig("speedup.png", dpi=200)

    # ---- Grafico de Eficiencia ----
    fig, ax = plt.subplots(figsize=(7, 5))
    ax.axhline(1.0, color="k", linestyle="--", linewidth=1,
               label="Eficiencia ideal (1,0)")
    for (nome, tempos), m in zip(series.items(), MARCADORES):
        _, e = calcula(tempos)
        ax.plot(THREADS, e, marker=m, linewidth=1.8, label=nome)
    ax.set_xlabel("Numero de threads (p)")
    ax.set_ylabel("Eficiencia  $E_p = S_p / p$")
    ax.set_title("Eficiencia x Numero de Threads")
    ax.set_xticks(THREADS)
    ax.set_ylim(0, 1.15)
    ax.grid(True, linestyle=":", alpha=0.6)
    ax.legend()
    fig.tight_layout()
    fig.savefig("eficiencia.png", dpi=200)

    print("Gerados: speedup.png e eficiencia.png")


if __name__ == "__main__":
    main()
