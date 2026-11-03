# Lab Computação Paralela — OpenMP `sections` e `tasks`

Três atividades sobre as diretivas `#pragma omp sections` e `#pragma omp task`.
Cada atividade tem a versão inicial (do enunciado) e a versão modificada.

## Arquivos

| Atividade | Versão inicial | Versão modificada | Extra |
|---|---|---|---|
| 1 — Análise estatística (sections) | `analise_inicial.c` | `analise_sections.c` | `analise_fundido.c` (laço único, só para comparação) |
| 2 — Partículas (sections) | `simulacao_inicial.c` | `simulacao_sections.c` | `-DINGENUO` compila a versão com condição de corrida |
| 3 — Contagem de linhas (tasks) | `contalinhas_inicial.c` | `contalinhas_tasks.c` | `-DUSAR_FGETC` volta ao `fgetc` original |

Apoio: `bench.sh` (medição), `resumo.py` (média/desvio), `resultados.csv` (medições),
`gerar_relatorio.py` (gera `relatorio.pdf`), `saida_a*.txt` (saídas de execução).

## Compilação

```bash
gcc -O3 -fopenmp analise_inicial.c    -o analise_inicial    -lm
gcc -O3 -fopenmp analise_sections.c   -o analise_sections   -lm
gcc -O3 -fopenmp analise_fundido.c    -o analise_fundido    -lm

gcc -O3 -fopenmp simulacao_inicial.c  -o simulacao_inicial
gcc -O3 -fopenmp simulacao_sections.c -o simulacao_sections
gcc -O3 -fopenmp -DINGENUO simulacao_sections.c -o simulacao_ingenua   # demonstra a corrida

gcc -O3 -fopenmp contalinhas_inicial.c -o contalinhas_inicial
gcc -O3 -fopenmp contalinhas_tasks.c   -o contalinhas_tasks
```

Requer GCC com OpenMP e Linux/glibc (usa `scandir`, `asprintf`, `getc_unlocked`).
A Atividade 1 usa ~400 MB de RAM; a Atividade 2, ~600 MB.

## Execução

```bash
OMP_NUM_THREADS=4 ./analise_sections
OMP_NUM_THREADS=4 ./simulacao_sections
OMP_NUM_THREADS=4 ./contalinhas_tasks <diretorio> [cutoff_em_bytes]   # padrão: ".", cutoff 0
```

## Medição

```bash
echo "programa,threads,execucao,tempo_s" > resultados.csv
./bench.sh ./analise_sections 5 "1 2 4 8" >> resultados.csv     # 5 execuções por nº de threads
./bench.sh ./contalinhas_tasks 5 "1 2 4 8" <diretorio> >> resultados.csv
python3 resumo.py resultados.csv
```

O `bench.sh` faz uma execução de aquecimento (descartada) antes de cada conjunto.
Use o mesmo diretório nas versões inicial e `tasks` da Atividade 3 e rode-as sem
outros programas pesados em paralelo.

## Gerar o relatório

O relatório editável está em `relatorio.docx` (Word); `relatorio.pdf` é a mesma versão em PDF. Para o Word, edite diretamente o arquivo (nomes na capa, tabelas e os parágrafos “Resultado medido (1 vCPU)”). Para regerar a partir de `resultados.csv`: `node gerar_docx.js` (requer o pacote npm `docx`). Para o PDF:

Edite `ALUNO` e `MEMBROS_GRUPO` no início de `gerar_relatorio.py` e rode:

```bash
pip install reportlab
python3 gerar_relatorio.py     # lê resultados.csv e saida_a*.txt, gera relatorio.pdf
```

As tabelas se atualizam sozinhas, mas os parágrafos “Resultado medido (1 vCPU)” são texto fixo escrito para o ambiente de teste: se você medir em outra máquina, reescreva-os com os seus números.

## Decisões de projeto (resumo)

- **Atividade 1:** índice dos laços declarado dentro de cada `section` (um `i` compartilhado seria corrida); reduções na diretiva `parallel sections`.
- **Atividade 2:** a seção de velocidade grava em um buffer (`vy_novo`) copiado depois, para a posição usar a velocidade antiga. `rand()` foi trocado por um gerador determinístico e thread-safe (splitmix64) nas duas versões; um checksum permite verificar equivalência.
- **Atividade 3:** `atomic` por arquivo (em vez de `reduction`), `parallel` + `single`, sem `taskwait` (a barreira final da região paralela basta); `fullpath` pertence à task que o libera; cutoff opcional por tamanho de arquivo; `_GNU_SOURCE` para `asprintf`; `getc_unlocked` no lugar de `fgetc` (ver relatório).
