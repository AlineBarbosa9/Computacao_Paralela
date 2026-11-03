#!/usr/bin/env python3
"""Uso: python3 resumo.py resultados.csv  -> média, desvio e speedup por programa/threads."""
import csv, sys, statistics as st
from collections import defaultdict
d = defaultdict(list)
for row in csv.reader(open(sys.argv[1])):
    if row and row[0] != 'programa':
        d[(row[0], int(row[1]))].append(float(row[3]))
print(f"{'programa':32s} {'thr':>3s} {'media(s)':>9s} {'desvio':>8s} {'n':>2s}")
for (p, t), v in sorted(d.items()):
    print(f"{p:32s} {t:3d} {st.mean(v):9.4f} {(st.stdev(v) if len(v)>1 else 0):8.4f} {len(v):2d}")
