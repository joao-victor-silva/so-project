#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>
#include <time.h>

#define MAX_PROCESSES 100
#define MAX_QUEUE_SIZE 100

typedef enum {
    TESTE10,
    TESTE20,
    TESTE30
} ProcessoExecutavel;

typedef enum {
    PRONTO,
    EXECUTANDO,
    FINALIZADO
} EstadoProcesso;

typedef struct {
    int pid;
    int id;
    int prioridade;
    ProcessoExecutavel executavel;
    time_t tempo_inicio;
    time_t tempo_fim;
    time_t ultima_execucao;
    long tempo_execucao;
    EstadoProcesso estado;
} EntradaTabela;

typedef struct {
    EntradaTabela entradas[MAX_PROCESSES];
    int num_processos;
} TabelaProcessos;

typedef struct {
    int ids[MAX_QUEUE_SIZE];
    int inicio;
    int fim;
    int tamanho;
} Fila;

typedef struct {
    Fila filas_prioridade[4];
    int quantum;
    int total_cores;
    int cores_disponiveis;
    time_t tempo_inicio;
    int total_processos;
    TabelaProcessos tabela;
    int ordem[MAX_PROCESSES];
    int ordem_index;
} SchedulerShared;

const char *executavel_string(ProcessoExecutavel executavel);

SchedulerShared *inicializar_memoria_compartilhada(int numero_de_cores, int quantum, int processos, int *shm_id);
void destruir_memoria_compartilhada(int shm_id, SchedulerShared *scheduler);

bool adicionar_na_fila(Fila *fila, int id);
bool remover_da_fila(Fila *fila, int *id);

void adicionar_entrada_tabela(TabelaProcessos *tabela, int id, int , ProcessoExecutavel executavel);
EntradaTabela *obter_entrada_tabela(TabelaProcessos *tabela, int id);

void imprimir_filas(SchedulerShared *scheduler);
void imprimir_tabela_processos(TabelaProcessos *tabela);

#endif
