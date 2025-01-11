#include "scheduler.h"
#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

bool obter_proximo_processo(SchedulerShared *scheduler, int *id, int prioridade) {
    Fila *fila = &scheduler->filas_prioridade[prioridade];
    if (fila->tamanho <= 0) {
        return false;
    }
    return remover_da_fila(&scheduler->filas_prioridade[prioridade], id);
}

void executar_programa(EntradaTabela *entrada) {
    entrada->ultima_execucao = time(NULL);

    if (entrada->pid == -1) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Erro ao criar processo filho");
            return;
        }

        if (pid == 0) {
            char file[50];
            strcpy(file, "./");
            strcat(file, executavel_string(entrada->executavel));
            execlp(file, file, NULL);
            perror("Erro ao executar o programa");
            _exit(EXIT_FAILURE);
        } else {
            entrada->pid = pid;
            entrada->estado = EXECUTANDO;
        }
    } else {
        // printf("Retomando execução do processo %d.\n", entrada->id);
        kill(entrada->pid, SIGCONT);
    }
}

void executar_processo(SchedulerShared *scheduler, pid_t processos_em_execucao[scheduler->total_cores]) {
    for (int prioridade = 0; prioridade < 4; prioridade++) {
        int id;

        if (obter_proximo_processo(scheduler, &id, prioridade)) {
            EntradaTabela *processo = obter_entrada_tabela(&scheduler->tabela, id);

            // printf("Executando processo %d (%s) com prioridade %d no tempo %ld.\n",
            //        processo->id, processo->executavel, processo->prioridade, time(NULL) - scheduler->tempo_inicio);
            executar_programa(processo);
            for (int i = 0; i < scheduler->total_cores; i++) {
                if (processos_em_execucao[i] == -1) {
                    processos_em_execucao[i] = id;
                    scheduler->cores_disponiveis--;
                    break;
                }
            }
            break;
        }
    }
}

void checar_processos(SchedulerShared *scheduler, pid_t processos_em_execucao[scheduler->total_cores], int *processos_executados) {
    for (int i = 0; i < scheduler->total_cores; i++) {
        if (processos_em_execucao[i] != -1) {
            int status;
            EntradaTabela *processo = obter_entrada_tabela(&scheduler->tabela, processos_em_execucao[i]);
            pid_t result = waitpid(processo->pid, &status, WNOHANG);
            if (time(NULL) - processo->ultima_execucao >= scheduler->quantum && result == 0) {
                // Processo ainda está executando
                kill(processo->pid, SIGSTOP);
                processo->tempo_execucao += time(NULL) - processo->ultima_execucao;
                adicionar_na_fila(&scheduler->filas_prioridade[processo->prioridade], processo->id);
                // scheduler->ordem[scheduler->ordem_index++] = processo->id;
                processos_em_execucao[i] = -1;
                scheduler->cores_disponiveis++;
            } else if (result != 0) {
                // Processo terminou completamente
                processo->tempo_execucao += time(NULL) - processo->ultima_execucao;
                processo->tempo_fim = time(NULL);
                processo->estado = FINALIZADO;
                // printf("Processo %d finalizado.", processo->id);
                (*processos_executados)++;
                scheduler->ordem[scheduler->ordem_index++] = processo->id;
                processos_em_execucao[i] = -1;
                scheduler->cores_disponiveis++;
            }
        }
    }
}


void executar_scheduler(SchedulerShared *scheduler) {
    printf("Iniciando o escalonador com %d cores e quantum de %d segundos.\n",
           scheduler->total_cores, scheduler->quantum);

    int processos_executados = 0;
    pid_t processos_em_execucao[scheduler->total_cores];

    for (int i = 0; i < scheduler->total_cores; i++) {
        processos_em_execucao[i] = -1;
    }

    while (scheduler->total_processos - processos_executados > 0) {
        if (scheduler->tabela.num_processos == 0) {
            continue;
        }
        if (scheduler->cores_disponiveis > 0) {
            executar_processo(scheduler, processos_em_execucao);
        }
        checar_processos(scheduler, processos_em_execucao, &processos_executados);
    }

    printf("Todos os processos foram executados. Finalizando o escalonador.\n");
}
