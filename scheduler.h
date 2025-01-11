#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>

#define MAX_PROCESSES 100
#define MAX_QUEUE_SIZE 100

typedef enum {
    PRONTO,
    EXECUTANDO,
    FINALIZADO
} EstadoProcesso;

typedef struct {
    int pid;
    int id;
    int prioridade;
    char *executavel;
    time_t tempo_inicio;
    time_t tempo_fim;
    time_t tempo_execucao;
    EstadoProcesso estado;
} EntradaTabela;

// Estrutura da tabela de processos
typedef struct {
    EntradaTabela entradas[MAX_PROCESSES];
    int num_processos;
} TabelaProcessos;

// Estrutura para representar uma fila circular de IDs
typedef struct {
    int ids[MAX_QUEUE_SIZE];
    int inicio;
    int fim;
    int tamanho;
} Fila;

// Estrutura do Scheduler em memória compartilhada
typedef struct {
    Fila filas_prioridade[4];  // 4 filas para prioridades 0, 1, 2, 3
    int quantum;               // Quantum de tempo
    int total_cores;           // Número total de cores
    int cores_disponiveis;     // Cores disponíveis
    long tempo_inicio;        // Tempo atual
    int total_processos;
    TabelaProcessos tabela;    // Tabela de processos
    int ordem[MAX_PROCESSES];
    int ordem_index;
} SchedulerShared;

void executar_scheduler(SchedulerShared *scheduler);

#endif // SCHEDULER_H
