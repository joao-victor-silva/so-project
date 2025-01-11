#include "scheduler.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SHM_KEY 200015

// Inicializar fila circular
void inicializar_fila(Fila *fila) {
    fila->inicio = 0;
    fila->fim = 0;
    fila->tamanho = 0;
}

// Função para inicializar a memória compartilhada
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

    // Inicializa as filas de prioridade
    for (int i = 0; i < 4; i++) {
        inicializar_fila(&scheduler->filas_prioridade[i]);
    }

    // Inicializa a tabela de processos e outros atributos do Scheduler
    scheduler->quantum = quantum;
    scheduler->total_cores = numero_de_cores;
    scheduler->cores_disponiveis = numero_de_cores;
    scheduler->tempo_inicio = time(NULL);
    scheduler->total_processos = total_processos;
    scheduler->tabela.num_processos = 0;
    scheduler->ordem_index = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        scheduler->ordem[i] = -1; // -1 indica posição vazia
    }
    return scheduler;
}

// Função para destruir a memória compartilhada
void destruir_memoria_compartilhada(int shm_id, SchedulerShared *scheduler) {
    if (shmdt(scheduler) == -1) {
        perror("Erro ao desassociar memória compartilhada");
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("Erro ao remover memória compartilhada");
    }
}

// Adicionar ID à fila
bool adicionar_fila(Fila *fila, int id) {
    if (fila->tamanho >= MAX_QUEUE_SIZE) {
        fprintf(stderr, "Fila cheia! Não é possível adicionar o ID.\n");
        return false;
    }
    fila->ids[fila->fim] = id;
    fila->fim = (fila->fim + 1) % MAX_QUEUE_SIZE;
    fila->tamanho++;
    return true;
}

// Remover ID da fila
bool remover_fila(Fila *fila, int *id) {
    if (fila->tamanho <= 0) {
        fprintf(stderr, "Fila vazia! Não é possível remover o ID.\n");
        return false;
    }
    *id = fila->ids[fila->inicio];
    fila->inicio = (fila->inicio + 1) % MAX_QUEUE_SIZE;
    fila->tamanho--;
    return true;
}

void adicionar_entrada_tabela(TabelaProcessos *tabela, int id, int prioridade, const char *executavel) {
    printf("Executavel de entrada tabela: %s\n", executavel);
    EntradaTabela *entrada = &tabela->entradas[tabela->num_processos++];
    entrada->pid = -1;
    entrada->id = id;
    entrada->prioridade = prioridade;
    // entrada->executavel = malloc(strlen(executavel) + 1);
    // strcpy(entrada->executavel, executavel);
    entrada->executavel = "teste10";
    entrada->tempo_inicio = time(NULL);
    entrada->tempo_fim = -1; // Ainda não finalizado
    entrada->estado = PRONTO;
}

EntradaTabela *obter_entrada_tabela(TabelaProcessos *tabela, int id) {
    for (int i = 0; i < tabela->num_processos; i++) {
        if (tabela->entradas[i].id == id) {
            return &tabela->entradas[i];
        }
    }
    return NULL; // Entrada não encontrada
}
