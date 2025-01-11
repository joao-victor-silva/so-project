#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H
#include "shared_memory.h"

typedef struct {
    int id;
    int prioridade;
    int tempo_inicio;
    ProcessoExecutavel executavel;
} Processo;

typedef struct TempoProcessos {
    int tempo_inicio;
    Processo processos[MAX_PROCESSES];
    int num_processos;
    struct TempoProcessos *proximo;
} TempoProcessos;

typedef struct {
    TempoProcessos *inicio;
    int total_processos;
} GerenciadorProcessos;

void adicionar_processos(SchedulerShared *scheduler, GerenciadorProcessos *gerenciador);
GerenciadorProcessos carregar_processos(const char *nome_arquivo);
void imprimir_gerenciador(GerenciadorProcessos *gerenciador);

#endif
