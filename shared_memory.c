#include "scheduler.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SHM_KEY 200015

const char *executavel_string(ProcessoExecutavel executavel) {
    switch (executavel) {
        case TESTE10:
            return "teste10";
        case TESTE20:
            return "teste20";
        case TESTE30:
            return "teste30";
    }
}

void inicializar_fila(Fila *fila) {
    fila->inicio = 0;
    fila->fim = 0;
    fila->tamanho = 0;
}

SchedulerShared *inicializar_memoria_compartilhada(int numero_de_cores, int quantum, int total_processos, int *shm_id) {
    *shm_id = shmget(SHM_KEY, sizeof(SchedulerShared), IPC_CREAT | 0666);
    if (*shm_id < 0) {
        perror("Erro ao criar memória compartilhada");
        exit(EXIT_FAILURE);
    }

    SchedulerShared *scheduler = (SchedulerShared *)shmat(*shm_id, NULL, 0);
    if (scheduler == (SchedulerShared *)-1) {
        perror("Erro ao associar memória compartilhada");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; i++) {
        inicializar_fila(&scheduler->filas_prioridade[i]);
    }

    scheduler->quantum = quantum;
    scheduler->total_cores = numero_de_cores;
    scheduler->cores_disponiveis = numero_de_cores;
    scheduler->tempo_inicio = time(NULL);
    scheduler->total_processos = total_processos;
    scheduler->tabela.num_processos = 0;
    scheduler->ordem_index = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        scheduler->ordem[i] = -1;
    }
    return scheduler;
}

void destruir_memoria_compartilhada(int shm_id, SchedulerShared *scheduler) {
    if (shmdt(scheduler) == -1) {
        perror("Erro ao desassociar memória compartilhada");
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("Erro ao remover memória compartilhada");
    }
}

bool adicionar_na_fila(Fila *fila, int id) {
    if (fila->tamanho >= MAX_QUEUE_SIZE) {
        fprintf(stderr, "Fila cheia! Não é possível adicionar o ID.\n");
        return false;
    }
    fila->ids[fila->fim] = id;
    fila->fim = (fila->fim + 1) % MAX_QUEUE_SIZE;
    fila->tamanho++;
    return true;
}

bool remover_da_fila(Fila *fila, int *id) {
    if (fila->tamanho <= 0) {
        fprintf(stderr, "Fila vazia! Não é possível remover o ID.\n");
        return false;
    }
    *id = fila->ids[fila->inicio];
    fila->inicio = (fila->inicio + 1) % MAX_QUEUE_SIZE;
    fila->tamanho--;
    return true;
}

void adicionar_entrada_tabela(TabelaProcessos *tabela, int id, int prioridade, ProcessoExecutavel executavel) {
    EntradaTabela *entrada = &tabela->entradas[tabela->num_processos++];
    entrada->pid = -1;
    entrada->id = id;
    entrada->prioridade = prioridade;
    entrada->executavel = executavel;
    entrada->tempo_inicio = time(NULL);
    entrada->tempo_fim = -1;
    entrada->estado = PRONTO;
    entrada->tempo_execucao = 0;
}

EntradaTabela *obter_entrada_tabela(TabelaProcessos *tabela, int id) {
    for (int i = 0; i < tabela->num_processos; i++) {
        if (tabela->entradas[i].id == id) {
            return &tabela->entradas[i];
        }
    }
    return NULL;
}

void imprimir_filas(SchedulerShared *scheduler) {
    printf("\nEstado atual das filas de prioridade:\n");
    for (int prioridade = 0; prioridade < 4; prioridade++) {
        printf("Fila de prioridade %d: ", prioridade);
        Fila *fila = &scheduler->filas_prioridade[prioridade];
        if (fila->tamanho == 0) {
            printf("vazia\n");
        } else {
            for (int i = 0; i < fila->tamanho; i++) {
                int index = (fila->inicio + i) % MAX_QUEUE_SIZE;
                printf("%d ", fila->ids[index]);
            }
            printf("\n");
        }
    }
    printf("\n");
}

void imprimir_tabela_processos(TabelaProcessos *tabela) {
    printf("\nTabela de Processos: (%d)\n", tabela->num_processos);
    printf("%-5s %-5s %-12s %-12s %-12s %-10s %-10s %-10s\n", "ID", "PID", "Prioridade", "Executável", "Tempo Início", "Tempo Fim", "Execução", "Estado");

    for (int i = 0; i < tabela->num_processos; i++) {
        EntradaTabela *entrada = &tabela->entradas[i];
        printf("%-5d %-5d %-12d %-12s %-12ld %-10ld %-10ld ",
               entrada->id,
               entrada->pid,
               entrada->prioridade,
               executavel_string(entrada->executavel),
               entrada->tempo_inicio,
               entrada->tempo_fim,
               entrada->tempo_execucao);

        switch (entrada->estado) {
            case PRONTO:
                printf("%-10s\n", "PRONTO");
            break;
            case EXECUTANDO:
                printf("%-10s\n", "EXECUTANDO");
            break;
            case FINALIZADO:
                printf("%-10s\n", "FINALIZADO");
            break;
        }
    }
    printf("\n");
}
