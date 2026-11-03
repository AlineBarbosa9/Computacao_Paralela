#define _GNU_SOURCE // asprintf (GNU); também cobre scandir/strdup
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <omp.h>

// Acumulador global compartilhado entre as tasks (atualizado com atomic).
static long long g_total_lines = 0;

// Cutoff opcional (bytes): arquivos menores que isso são contados na própria
// task que os encontrou, sem criar uma task extra (evita overhead de tasks
// minúsculas). Com 0 (padrão), TODO arquivo .c/.h vira uma task, como no enunciado.
static long long g_file_cutoff = 0;

// Leitura de caractere. fgetc() adquire o lock interno do FILE* a cada chamada
// assim que o processo tem mais de uma thread (o que ocorre ao entrar em uma
// região parallel), o que degrada muito o desempenho. Como cada FILE* aqui é
// local a uma única chamada de count_lines_in_file, getc_unlocked() é seguro.
// Compile com -DUSAR_FGETC para reproduzir o comportamento original.
#ifdef USAR_FGETC
  #define LER_CHAR(fp) fgetc(fp)
#else
  #define LER_CHAR(fp) getc_unlocked(fp)
#endif

// Função para contar linhas em um único arquivo (inalterada)
long long count_lines_in_file(const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return 0; // Ignora arquivos que não pode abrir
    }
    long long count = 0;
    int ch;
    while ((ch = LER_CHAR(fp)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }
    fclose(fp);
    return count;
}

// Conta o arquivo e soma o resultado ao acumulador global (uma única
// atualização atômica por arquivo, o que mantém a contenção baixa).
static void process_file(char *fullpath) {
    long long lines = count_lines_in_file(fullpath);
    #pragma omp atomic
    g_total_lines += lines;
    free(fullpath);
}

// Percorre um diretório. Cada subdiretório e cada arquivo .c/.h vira uma task.
// A função não retorna contagem: os resultados chegam via g_total_lines.
// Não há taskwait: a barreira implícita ao final da região parallel garante
// que todas as tasks (inclusive as criadas por outras tasks) terminaram.
void traverse_directory(const char *dirpath) {
    struct dirent **namelist;
    int n = scandir(dirpath, &namelist, NULL, alphasort);
    if (n < 0) {
        fprintf(stderr, "Erro ao escanear diretório %s: %s\n", dirpath, strerror(errno));
        return;
    }

    for (int i = 0; i < n; i++) {
        // Ignora "." e ".."
        if (strcmp(namelist[i]->d_name, ".") == 0
            || strcmp(namelist[i]->d_name, "..") == 0) {
            free(namelist[i]);
            continue;
        }

        char *fullpath = NULL;
        if (asprintf(&fullpath, "%s/%s", dirpath, namelist[i]->d_name) == -1) {
            fprintf(stderr, "Erro de alocação em asprintf\n");
            free(namelist[i]);
            continue;
        }

        struct stat path_stat;
        if (stat(fullpath, &path_stat) != 0) {
            fprintf(stderr, "Erro no stat para %s: %s\n", fullpath, strerror(errno));
            free(fullpath);
            free(namelist[i]);
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            // A task assume a posse de fullpath (firstprivate copia o ponteiro)
            // e é quem o libera. namelist[i] não é usado pela task.
            #pragma omp task firstprivate(fullpath)
            {
                traverse_directory(fullpath);
                free(fullpath);
            }
        }
        else if (S_ISREG(path_stat.st_mode)) {
            size_t len = strlen(namelist[i]->d_name);
            if (len > 2 && (strcmp(namelist[i]->d_name + len - 2, ".c") == 0 ||
                            strcmp(namelist[i]->d_name + len - 2, ".h") == 0))
            {
                if ((long long)path_stat.st_size < g_file_cutoff) {
                    process_file(fullpath);  // pequeno: conta direto, sem task
                } else {
                    #pragma omp task firstprivate(fullpath)
                    process_file(fullpath);
                }
            } else {
                free(fullpath); // não é .c/.h
            }
        }
        else {
            free(fullpath); // nem diretório nem arquivo regular
        }
        free(namelist[i]);
    }
    free(namelist);
}

int main(int argc, char *argv[]) {
    const char *start_dir = ".";
    if (argc > 1) start_dir = argv[1];
    if (argc > 2) g_file_cutoff = atoll(argv[2]);

    printf("Contando linhas em .c/.h a partir de: %s (versão tasks, cutoff de arquivo = %lld bytes)\n",
           start_dir, g_file_cutoff);

    double start_time = omp_get_wtime();

    // --- Bloco otimizado ---
    // Uma thread (single) inicia a travessia e gera tasks; as demais threads
    // da equipe as executam. As próprias tasks geram novas tasks.
    #pragma omp parallel
    {
        #pragma omp single
        traverse_directory(start_dir);
    } // barreira implícita: aguarda TODAS as tasks
    // --- Fim do Bloco ---

    double end_time = omp_get_wtime();

    printf("\n--- Resultado ---\n");
    printf("Total de linhas encontradas: %lld\n", g_total_lines);
    printf("Tempo de execução: %.4f segundos\n", end_time - start_time);

    return 0;
}
