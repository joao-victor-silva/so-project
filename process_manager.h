#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H
#include "shared_memory.h"

typedef struct {
    int id;
    char executavel[50];
    int tempo_inicio;
    int prioridade;
} Processo;

typedef struct {
    int id;              // ID do processo
    int prioridade;      // Prioridade do processo
    int tempo_inicio;    // Tempo de início
    char executavel[50]; // Nome do executável
} ProcessoAgrupado;

typedef struct TempoProcessos {
    int tempo_inicio;                 // Tempo de início
    ProcessoAgrupado processos[MAX_PROCESSES]; // Lista de processos
    int num_processos;                // Quantidade de processos neste tempo
    struct TempoProcessos *proximo;   // Próximo grupo de tempos
} TempoProcessos;

typedef struct {
    TempoProcessos *inicio; // Início da lista encadeada
    int total_processos;
} GerenciadorProcessos;

// void adicionar_processos(SchedulerShared *scheduler, Processo processos[], int num_processos);
void adicionar_processos(SchedulerShared *scheduler, GerenciadorProcessos *gerenciador);
GerenciadorProcessos carregar_processos(const char *nome_arquivo);
void imprimir_gerenciador(GerenciadorProcessos *gerenciador);
void imprimir_tabela_processos(TabelaProcessos *tabela);

#endif //PROCESS_MANAGER_H
