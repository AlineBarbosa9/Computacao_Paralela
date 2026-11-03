#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Gera relatorio.pdf a partir de resultados.csv e saida_a*.txt.
Edite ALUNO / MEMBROS_GRUPO e rode: python3 gerar_relatorio.py
Parágrafos marcados com descrevem as medições do ambiente de teste (1 vCPU);
se você refizer as medições em outra máquina, atualize o texto conforme os novos números."""
import csv, os, platform, statistics as st, subprocess, datetime
from collections import defaultdict
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import cm
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.enums import TA_JUSTIFY
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (SimpleDocTemplate, Paragraph, Spacer, Preformatted,
                                Table, TableStyle, PageBreak, KeepTogether)

ALUNO = "[NOME DO ALUNO]"
MEMBROS_GRUPO = ["[Nome do membro 1]", "[Nome do membro 2]"]   # inclua você, se fizer parte do grupo
DISCIPLINA = "Computação Paralela — Lab OpenMP Tasks/Sections"

F = "/usr/share/fonts/truetype/dejavu/"
pdfmetrics.registerFont(TTFont("DV", F + "DejaVuSans.ttf"))
pdfmetrics.registerFont(TTFont("DV-B", F + "DejaVuSans-Bold.ttf"))
pdfmetrics.registerFont(TTFont("DV-I", F + "DejaVuSans-Oblique.ttf"))
pdfmetrics.registerFont(TTFont("DVM", F + "DejaVuSansMono.ttf"))
pdfmetrics.registerFontFamily("DV", normal="DV", bold="DV-B", italic="DV-I", boldItalic="DV-B")

ss = getSampleStyleSheet()
corpo = ParagraphStyle("corpo", parent=ss["Normal"], fontName="DV", fontSize=9.5, leading=14, alignment=TA_JUSTIFY, spaceAfter=6)
h1 = ParagraphStyle("h1", parent=corpo, fontName="DV-B", fontSize=14, leading=18, spaceBefore=10, spaceAfter=6, alignment=0)
h2 = ParagraphStyle("h2", parent=corpo, fontName="DV-B", fontSize=11, leading=15, spaceBefore=8, spaceAfter=4, alignment=0)
titulo = ParagraphStyle("t", parent=h1, fontSize=20, leading=26, alignment=1, spaceAfter=10)
centro = ParagraphStyle("c", parent=corpo, alignment=1)
nota = ParagraphStyle("n", parent=corpo, fontSize=8.5, leading=12, textColor=colors.HexColor("#444444"))
cod = ParagraphStyle("cod", fontName="DVM", fontSize=7.2, leading=9.2, backColor=colors.HexColor("#F3F3F3"),
                     borderPadding=4, spaceBefore=4, spaceAfter=8)

def P(t, s=corpo): return Paragraph(t, s)
def C(t): return Preformatted(t.strip("\n"), cod)
def lerf(n): return open(n, encoding="utf-8").read().strip("\n") if os.path.exists(n) else "(arquivo " + n + " não encontrado)"

# ---------- dados ----------
dados = defaultdict(list)
for r in csv.reader(open("resultados.csv")):
    if r and r[0] != "programa": dados[(r[0], int(r[1]))].append(float(r[3]))
def media(p, t): return st.mean(dados[(p, t)]) if (p, t) in dados else None
def fmt(p, t):
    v = dados.get((p, t))
    if not v: return "—"
    return f"{st.mean(v):.4f} ± {(st.stdev(v) if len(v) > 1 else 0):.4f}"
def tabela(progs, threads, rotulos):
    linhas = [["Versão"] + [f"{t} thr." for t in threads]]
    for p, rot in zip(progs, rotulos): linhas.append([rot] + [fmt(p, t) for t in threads])
    t = Table(linhas, hAlign="LEFT", colWidths=[5.2*cm] + [2.9*cm]*len(threads))
    t.setStyle(TableStyle([("FONT", (0,0), (-1,-1), "DV", 8), ("FONT", (0,0), (-1,0), "DV-B", 8),
        ("GRID", (0,0), (-1,-1), 0.4, colors.grey), ("BACKGROUND", (0,0), (-1,0), colors.HexColor("#E6E6E6")),
        ("ALIGN", (1,0), (-1,-1), "CENTER"), ("VALIGN", (0,0), (-1,-1), "MIDDLE")]))
    return KeepTogether([t])

def sh(c):
    try: return subprocess.run(c, shell=True, capture_output=True, text=True).stdout.strip()
    except Exception: return ""
cpu = sh("grep -m1 'model name' /proc/cpuinfo | cut -d: -f2").strip() or platform.processor()
ncpu = os.cpu_count()
gccv = sh("gcc --version | head -1")
so = sh("grep PRETTY_NAME /etc/os-release | cut -d= -f2").strip('"') or platform.platform()

S = []
# ---------- capa ----------
S += [Spacer(1, 3*cm), P("Relatório — Atividades OpenMP", titulo), P("<b>sections</b> e <b>tasks</b>", centro), Spacer(1, 1*cm),
      P(DISCIPLINA, centro), P("Universidade Presbiteriana Mackenzie — FCI", centro), Spacer(1, 1*cm),
      P(f"<b>Aluno (entrega individual):</b> {ALUNO}", centro),
      P("<b>Membros do grupo:</b> " + "; ".join(MEMBROS_GRUPO), centro), Spacer(1, 1*cm),
      P(datetime.date.today().strftime("%d/%m/%Y"), centro), PageBreak()]

# ---------- 1. metodologia ----------
S += [P("1. Introdução e metodologia", h1),
      P("Este relatório descreve as três atividades do roteiro: (1) análise estatística de um vetor com "
        "<font name='DVM'>#pragma omp sections</font>; (2) simulação de partículas com <font name='DVM'>sections</font>; "
        "(3) contagem de linhas de arquivos .c/.h com <font name='DVM'>#pragma omp task</font>. Para cada uma, comparo a "
        "versão inicial do roteiro com a versão modificada quanto a correção, clareza e desempenho."),
      P("<b>Ambiente de medição.</b>", corpo),
      P(f"CPU: {cpu}; CPUs lógicas visíveis: {ncpu}; SO: {so}; compilador: {gccv}. "
        "Compilação: <font name='DVM'>gcc -O3 -fopenmp</font> (e <font name='DVM'>-lm</font> quando necessário)."),
      P("<b>Procedimento.</b> Cada configuração (programa × número de threads via <font name='DVM'>OMP_NUM_THREADS</font>) "
        "tem uma execução de aquecimento descartada e 5 execuções medidas (2 nas variantes com <font name='DVM'>fgetc</font>, "
        "por serem lentas). O tempo vem de <font name='DVM'>omp_get_wtime()</font> e restringe-se ao “bloco a ser otimizado”. "
        "As tabelas mostram média ± desvio-padrão em segundos."),
      P("<b>Verificação de correção.</b> Antes de comparar tempos, confirmei que as versões produzem o mesmo resultado "
        "(Atividade 1: mesmos min/max/soma/desvio; Atividade 2: mesmo checksum de y e vy; Atividade 3: mesmo total de linhas). "
        "A versão com tasks também foi executada com AddressSanitizer/UBSan, sem erros nem vazamentos reportados.")]
if ncpu == 1:
    S.append(P("<b>Limitação importante:</b> o ambiente de medição expõe apenas <b>1 CPU lógica</b>. Com 1 CPU, usar mais threads "
               "gera concorrência, mas não paralelismo real; portanto <b>ganhos de speedup por paralelismo não podem ser "
               "observados nestes números</b>. Eles servem para comparar custo de trabalho total, overhead e corretude. "
               "Conclusões sobre escalabilidade em máquinas multicore aparecem identificadas como <i>expectativa teórica</i>, "
               "não como resultado medido.", nota))

# ---------- 2. Atividade 1 ----------
S += [P("2. Atividade 1 — Análise estatística (sections)", h1), P("2.1 Alterações realizadas", h2),
      P("Os três <font name='DVM'>parallel for</font> viraram uma única região <font name='DVM'>parallel sections</font> com três "
        "<font name='DVM'>section</font>: mínimo, máximo e soma/soma dos quadrados. As cláusulas "
        "<font name='DVM'>reduction(min:)</font>, <font name='DVM'>reduction(max:)</font> e <font name='DVM'>reduction(+:)</font> ficam "
        "na diretiva combinada."),
      C("""#pragma omp parallel sections reduction(min:min_val) reduction(max:max_val) \\
                              reduction(+:sum, sum_sq)
{
    #pragma omp section
    { for (long long k = 0; k < N; k++) if (data[k] < min_val) min_val = data[k]; }
    #pragma omp section
    { for (long long k = 0; k < N; k++) if (data[k] > max_val) max_val = data[k]; }
    #pragma omp section
    { for (long long k = 0; k < N; k++) { sum += data[k];
                                          sum_sq += (long long)data[k] * data[k]; } }
}"""),
      P("<b>Decisão de projeto: índice do laço.</b> Na versão inicial, <font name='DVM'>long long i</font> é declarado fora e o "
        "<font name='DVM'>parallel for</font> o torna privado automaticamente. Dentro de <font name='DVM'>sections</font> isso não acontece: "
        "um <font name='DVM'>i</font> declarado fora seria compartilhado e geraria condição de corrida. Por isso cada laço declara o seu próprio "
        "índice (<font name='DVM'>k</font>) dentro da seção."),
      P("<b>Decisão de projeto: reduções.</b> Cada variável é escrita por uma única seção, mas a redução continua sendo a forma correta "
        "de declarar o compartilhamento: cada thread recebe cópia privada inicializada com o elemento neutro (INT_MAX, INT_MIN, 0) e as cópias "
        "são combinadas no fim da região. Threads que não executam nenhuma seção contribuem apenas com o elemento neutro."),
      P("2.2 Clareza", h2),
      P("A versão com <font name='DVM'>sections</font> é mais explícita sobre a estrutura (três tarefas independentes, uma por métrica), "
        "mas é <b>mais verbosa</b> e <b>duplica a lógica de laço</b> três vezes, além de exigir cuidado com o índice e com o escopo das "
        "variáveis. A versão inicial é mais curta, porém dispersa a mesma ideia em três regiões paralelas separadas. Em termos de "
        "legibilidade, considero as duas equivalentes; a de <font name='DVM'>sections</font> é ligeiramente mais frágil a erros de escopo."),
      P("2.3 Desempenho e variação de threads", h2),
      tabela(["analise_inicial", "analise_sections", "analise_fundido"], [1, 2, 4, 8],
             ["Inicial (3 × parallel for)", "sections (1 região)", "Extra: laço único fundido"]), Spacer(1, 6),
      P("<b>Resultado medido (1 vCPU).</b> Inicial e <font name='DVM'>sections</font> ficaram próximas (≈ 0,21–0,25 s) e "
        "praticamente insensíveis ao número de threads, como esperado com uma única CPU: as diferenças entre elas estão dentro da "
        "variação entre execuções. Já a versão <b>fundida</b> (um único laço com todas as reduções) levou cerca de metade do tempo "
        "(≈ 0,115 s). Esse ganho <b>não depende de paralelismo</b>: o vetor de 400 MB é percorrido uma vez em vez de três, o que reduz "
        "o tráfego de memória, e o laço é memory-bound."),
      P("<i>Expectativa teórica em máquina multicore (não medida aqui):</i> com <font name='DVM'>sections</font>, no máximo 3 threads "
        "trabalham (uma por seção), e a seção da soma é a mais pesada e limita o tempo total; os três <font name='DVM'>parallel for</font> usam "
        "todas as threads em cada passada. Como cada versão ainda percorre o vetor três vezes e o problema tende a ser limitado pela largura "
        "de banda da memória, não espero que <font name='DVM'>sections</font> supere as versões com <font name='DVM'>for</font>; e o laço fundido deve "
        "continuar sendo a melhor opção. <font name='DVM'>sections</font> expressa paralelismo funcional fixo e não escala com o tamanho dos dados."),
      PageBreak()]

# ---------- 3. Atividade 2 ----------
S += [P("3. Atividade 2 — Simulação de partículas (sections)", h1), P("3.1 Problema encontrado no enunciado: condição de corrida", h2),
      P("O enunciado afirma que as duas atualizações são independentes. Na verdade, a seção de posição <b>lê</b> "
        "<font name='DVM'>particles[i].vy</font> e a seção de velocidade <b>escreve</b> em <font name='DVM'>particles[i].vy</font>. "
        "Executadas simultaneamente, a posição de y pode usar a velocidade já atualizada, o que difere da versão sequencial. "
        "Para comprovar, compilei também uma versão ingênua (<font name='DVM'>-DINGENUO</font>) que coloca os dois laços em duas seções sem "
        "nenhuma proteção, e comparei o checksum de y:"),
      ]
tc = Table([["Versão", "Threads", "Checksum y"],
            ["Inicial (sequencial por laços)", "2 e 4", "524334304.665046"],
            ["sections corrigida", "1, 2 e 4", "524334304.665046"],
            ["sections ingênua (corrida)", "1", "524334304.665046"],
            ["sections ingênua (corrida)", "2", "524324192.276733"],
            ["sections ingênua (corrida)", "4", "524324891.294171"]], hAlign="LEFT", colWidths=[6*cm, 2.5*cm, 4.5*cm])
tc.setStyle(TableStyle([("FONT", (0,0), (-1,-1), "DV", 8), ("FONT", (0,0), (-1,0), "DV-B", 8),
    ("GRID", (0,0), (-1,-1), 0.4, colors.grey), ("BACKGROUND", (0,0), (-1,0), colors.HexColor("#E6E6E6"))]))
S += [tc, Spacer(1, 6),
      P("A versão ingênua produz resultados <b>diferentes e variáveis</b> assim que há mais de uma thread (com 1 thread as seções "
        "executam em sequência e o resultado coincide). Isso mostra que a corrida é real, e não apenas teórica (checksums medidos no ambiente de teste)."),
      P("3.2 Alterações realizadas", h2),
      P("<b>Correção da corrida.</b> A seção de velocidade grava em um buffer separado (<font name='DVM'>vy_novo</font>); depois do fim das seções "
        "(barreira implícita), um <font name='DVM'>parallel for</font> copia o buffer para a estrutura. Assim, a seção de posição enxerga "
        "sempre a velocidade antiga, como na versão sequencial."),
      C("""double *vy_novo = malloc(NUM_PARTICLES * sizeof(double));
#pragma omp parallel sections
{
    #pragma omp section
    { for (long long k = 0; k < NUM_PARTICLES; k++) {
          particles[k].x += particles[k].vx * DT;   /* usa vy ANTIGO */
          particles[k].y += particles[k].vy * DT;
          particles[k].z += particles[k].vz * DT; } }
    #pragma omp section
    { for (long long k = 0; k < NUM_PARTICLES; k++)
          vy_novo[k] = particles[k].vy + GRAVITY * DT; }
}                                    /* barreira implícita */
#pragma omp parallel for             /* publica as novas velocidades */
for (i = 0; i < NUM_PARTICLES; i++) particles[i].vy = vy_novo[i];
free(vy_novo);"""),
      P("<b>Inicialização.</b> A versão inicial chama <font name='DVM'>rand()</font> dentro de <font name='DVM'>parallel for</font>; "
        "<font name='DVM'>rand()</font> mantém estado global (não é thread-safe) e torna o resultado dependente do escalonamento. "
        "Substituí-o, <b>nas duas versões</b>, por um gerador determinístico por índice (splitmix64), fora do bloco medido. Isso torna as "
        "execuções reprodutíveis e permite comparar versões por checksum. Os tempos medidos não incluem a inicialização."),
      P("3.3 Clareza", h2),
      P("A versão com <font name='DVM'>sections</font> é <b>mais difícil de entender</b> que a original: exige raciocinar sobre dependências de "
        "dados entre seções, um buffer auxiliar, a barreira implícita e uma cópia final. A versão inicial é trivial de verificar. Uma "
        "alternativa mais simples e livre de corrida seria fundir os dois laços em um só (<font name='DVM'>y += vy*DT; vy += g*DT;</font> na mesma "
        "iteração, nesta ordem); não a medi, apenas a registro como opção de projeto."),
      P("3.4 Desempenho e variação de threads", h2),
      tabela(["simulacao_inicial", "simulacao_sections"], [1, 2, 4, 8], ["Inicial (2 × parallel for)", "sections (corrigida)"]),
      Spacer(1, 6),
      P("<b>Resultado medido (1 vCPU).</b> A versão <font name='DVM'>sections</font> corrigida foi cerca de 1,8 vez <b>mais lenta</b> "
        "(≈ 0,20–0,22 s contra ≈ 0,115 s), independentemente do número de threads. Como só há uma CPU, não há paralelismo a ganhar; "
        "o que aparece é o <b>custo extra da correção</b>: alocação e primeiro acesso a 80 MB de buffer dentro do trecho medido, uma passada "
        "adicional de cópia, e laços que percorrem a estrutura de 48 bytes por partícula em dois fluxos separados. Não medi cada componente "
        "isoladamente."),
      P("<i>Expectativa teórica em máquina multicore (não medida aqui):</i> <font name='DVM'>sections</font> só consegue usar 2 threads, enquanto "
        "os <font name='DVM'>parallel for</font> usam todas. Como os laços são limitados por memória (poucas operações aritméticas por byte), o "
        "ganho de escalabilidade de qualquer versão tende a saturar na largura de banda; o limite de 2 threads de <font name='DVM'>sections</font> e o "
        "custo do buffer tornam improvável que ela supere a versão inicial."),
      PageBreak()]

# ---------- 4. Atividade 3 ----------
S += [P("4. Atividade 3 — Contagem de linhas (tasks)", h1), P("4.1 Alterações realizadas", h2),
      C("""static long long g_total_lines = 0;           /* acumulador global */

static void process_file(char *fullpath) {
    long long lines = count_lines_in_file(fullpath);
    #pragma omp atomic
    g_total_lines += lines;                     /* 1 atomic por arquivo */
    free(fullpath);
}
void traverse_directory(const char *dirpath) {
    ...
    if (S_ISDIR(path_stat.st_mode)) {
        #pragma omp task firstprivate(fullpath)
        { traverse_directory(fullpath); free(fullpath); }
    } else if (S_ISREG(...) && (termina em .c ou .h)) {
        if (st_size < g_file_cutoff) process_file(fullpath);   /* cutoff opcional */
        else {
            #pragma omp task firstprivate(fullpath)
            process_file(fullpath);
        }
    } ...
}
/* main */
#pragma omp parallel
{
    #pragma omp single
    traverse_directory(start_dir);
}                                    /* barreira implícita: aguarda todas as tasks */"""),
      P("<b>Soma dos resultados: atomic em vez de reduction.</b> Tasks criadas dinamicamente e recursivamente não se encaixam em "
        "<font name='DVM'>reduction</font> sem <font name='DVM'>taskloop</font>/<font name='DVM'>taskgroup</font>. Usei um acumulador global com "
        "<font name='DVM'>#pragma omp atomic</font>, atualizado <b>uma vez por arquivo</b> (o contador local é calculado sem sincronização), o que mantém "
        "a contenção baixa. Por isso <font name='DVM'>traverse_directory</font> passou a retornar <font name='DVM'>void</font>."),
      P("<b>Sincronização.</b> Não há <font name='DVM'>taskwait</font>: a barreira implícita no fim da região <font name='DVM'>parallel</font> só é "
        "liberada quando todas as tasks pendentes, inclusive as criadas por outras tasks, terminam. Um <font name='DVM'>taskwait</font> seria "
        "necessário apenas se alguma task precisasse do resultado de suas filhas."),
      P("<b>Posse de memória.</b> <font name='DVM'>fullpath</font> é alocado com <font name='DVM'>asprintf</font> e precisa sobreviver à iteração do laço "
        "que cria a task. Com <font name='DVM'>firstprivate(fullpath)</font> a task recebe uma cópia do <i>ponteiro</i> e passa a ser dona da memória "
        "(é ela quem chama <font name='DVM'>free</font>); nos demais caminhos o próprio laço libera. <font name='DVM'>namelist[i]</font> não é usado dentro "
        "das tasks, portanto pode ser liberado no laço."),
      P("<b>Cutoff.</b> Criar uma task por arquivo minúsculo pode custar mais que o trabalho em si. Adicionei um cutoff opcional por tamanho "
        "(<font name='DVM'>argv[2]</font>, em bytes): arquivos menores são contados inline. O padrão é 0, ou seja, uma task por arquivo, como no enunciado."),
      P("<b>Portabilidade.</b> <font name='DVM'>asprintf</font> é extensão GNU e exige <font name='DVM'>_GNU_SOURCE</font>; o "
        "<font name='DVM'>_XOPEN_SOURCE 700</font> do código inicial sozinho pode não declará-la."),
      P("4.2 Descoberta importante: o lock do fgetc", h2),
      P("Na primeira medição, a versão com tasks levou ≈ 13 s com 2 ou mais threads, contra ≈ 2,6 s com 1 thread e na versão sequencial — "
        "uma lentidão de 5 vezes que <b>não</b> vinha das tasks. Testei duas hipóteses: espera ativa das threads ociosas "
        "(<font name='DVM'>OMP_WAIT_POLICY=passive</font>: sem efeito) e o lock interno do <font name='DVM'>FILE*</font>. A glibc só adquire o lock "
        "em <font name='DVM'>fgetc</font> quando o processo possui mais de uma thread — o que passa a valer quando o OpenMP cria sua equipe. Como "
        "<font name='DVM'>count_lines_in_file</font> chama <font name='DVM'>fgetc</font> uma vez por byte (centenas de milhões de vezes), o custo se "
        "acumula. Substituí por <font name='DVM'>getc_unlocked</font>, que é seguro aqui porque cada <font name='DVM'>FILE*</font> é local a uma única chamada. "
        "O tempo caiu de ≈ 13 s para ≈ 0,9 s. Apliquei a mesma troca na versão inicial para manter a comparação justa; "
        "<font name='DVM'>-DUSAR_FGETC</font> reproduz o comportamento original."),
      P("4.3 Desempenho e variação de threads", h2),
      tabela(["contalinhas_inicial", "contalinhas_tasks", "contalinhas_tasks_cutoff64k"], [1, 2, 4, 8],
             ["Inicial (sequencial)", "tasks (cutoff 0)", "tasks (cutoff 64 KB)"]), Spacer(1, 6),
      tabela(["contalinhas_inicial_fgetc", "contalinhas_tasks_fgetc"], [1, 4], ["Inicial com fgetc original", "tasks com fgetc original"]),
      Spacer(1, 6),
      P("Árvore de teste: 8 cópias de <font name='DVM'>/usr/include</font> (42.808 arquivos .c/.h; 15.218.008 linhas), com cache de arquivos quente "
        "após o aquecimento. Todas as versões reportaram o mesmo total.", nota),
      P("<b>Resultado medido (1 vCPU).</b> Com <font name='DVM'>getc_unlocked</font>, sequencial e tasks ficaram praticamente iguais "
        "(≈ 0,84–0,94 s), sem diferença acima da variação entre execuções, inclusive com 8 threads (oversubscrição) e com o cutoff. Ou seja, "
        "<b>o overhead de criar e escalonar ≈ 43 mil tasks é pequeno</b> frente ao trabalho de contar bytes, mas, com uma CPU, também não há ganho. "
        "Com o <font name='DVM'>fgetc</font> original, a versão com 4 threads foi ≈ 5 vezes mais lenta que com 1 (13,5 s contra 2,7 s)."),
      P("<i>Expectativa teórica em máquina multicore (não medida aqui):</i> com cache quente o trabalho é CPU-bound e deve escalar com o número "
        "de núcleos até o limite do paralelismo disponível (diretórios e arquivos grandes). Com cache frio, o gargalo passa a ser o disco. "
        "Arquivos muito desiguais (alguns enormes) limitam o speedup pelo caminho mais longo. O cutoff reduz overhead quando há muitos arquivos "
        "pequenos."),
      P("4.4 Clareza", h2),
      P("A versão com tasks é <b>mais difícil de entender e de manter</b>: exige raciocinar sobre quem libera cada ponteiro, sobre a ordem de "
        "conclusão não determinística, sobre a variável global e sobre a barreira final. A versão sequencial é natural e recursiva. Em troca, "
        "tasks expressam bem um paralelismo dinâmico e irregular, em que o número de unidades de trabalho só é conhecido durante a execução — "
        "algo que <font name='DVM'>sections</font> e <font name='DVM'>parallel for</font> não conseguem representar diretamente."),
      PageBreak()]

# ---------- 5. Prints ----------
S += [P("5. Saídas de execução", h1),
      P("As saídas abaixo foram geradas pelos programas com <font name='DVM'>OMP_NUM_THREADS=4</font>.", nota),
      P("Atividade 1", h2), C(lerf("saida_a1.txt")),
      P("Atividade 2", h2), C(lerf("saida_a2.txt")),
      P("Atividade 3", h2), C(lerf("saida_a3.txt")), PageBreak()]

# ---------- 6. Conclusão ----------
S += [P("6. Conclusão", h1),
      P("<b>sections</b> serve ao paralelismo funcional <i>fixo</i>: um número pequeno e conhecido de blocos independentes. Seu limite de "
        "paralelismo é o número de seções, e ele não escala com o tamanho do problema; nas Atividades 1 e 2 isso, somado ao fato de os laços "
        "serem limitados por memória, o torna pouco vantajoso. Na Atividade 2 ainda foi preciso <b>corrigir uma dependência de dados que o "
        "enunciado dava como inexistente</b>, com custo adicional."),
      P("<b>tasks</b> servem ao paralelismo <i>dinâmico e recursivo</i>, como a travessia de diretórios. O preço é a complexidade: gerenciamento de "
        "memória entre tasks, escolha da forma de acumular resultados (<font name='DVM'>atomic</font> por arquivo) e atenção ao overhead e a cutoffs."),
      P("Dois aprendizados gerais: (i) a melhor abordagem depende da estrutura do problema — nas Atividades 1 e 2 a opção simples (laço fundido) "
        "tende a vencer; (ii) a corretude e a verificação vêm antes do desempenho — checksums revelaram a corrida da Atividade 2, e o "
        "gargalo da Atividade 3 estava na biblioteca de E/S, não no OpenMP."),
      P("<b>Arquivos entregues:</b> analise_inicial.c, analise_sections.c, analise_fundido.c, simulacao_inicial.c, simulacao_sections.c, "
        "contalinhas_inicial.c, contalinhas_tasks.c, bench.sh, resumo.py, resultados.csv, README.md, gerar_relatorio.py.")]

doc = SimpleDocTemplate("relatorio.pdf", pagesize=A4, leftMargin=2.2*cm, rightMargin=2.2*cm, topMargin=2*cm, bottomMargin=2*cm,
                        title="Relatório OpenMP sections/tasks", author=ALUNO)
def rodape(c, d):
    c.saveState(); c.setFont("DV", 8); c.drawCentredString(A4[0]/2, 1.1*cm, f"{d.page}"); c.restoreState()
doc.build(S, onFirstPage=lambda c, d: None, onLaterPages=rodape)
print("relatorio.pdf gerado")
