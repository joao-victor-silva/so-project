#include "process_manager.h"
#include "scheduler.h"
#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void inicializar_gerenciador(GerenciadorProcessos *gerenciador) {
    gerenciador->inicio = NULL;
}

void adicionar_processo_ao_gerenciador(GerenciadorProcessos *gerenciador, Processo *processo) {
    TempoProcessos *atual = gerenciador->inicio;
    TempoProcessos *anterior = NULL;

    // Busca ou cria o grupo para o tempo de início
    while (atual != NULL && atual->tempo_inicio < processo->tempo_inicio) {
        anterior = atual;
        atual = atual->proximo;
    }

    if (atual == NULL || atual->tempo_inicio != processo->tempo_inicio) {
        // Cria um novo grupo para este tempo
        TempoProcessos *novo = (TempoProcessos *)malloc(sizeof(TempoProcessos));
        novo->tempo_inicio = processo->tempo_inicio;
        novo->num_processos = 0;
        novo->proximo = atual;

        if (anterior == NULL) {
            gerenciador->inicio = novo;
        } else {
            anterior->proximo = novo;
        }

        atual = novo;
    }

    // Adiciona o processo na lista ordenada por prioridade
    int pos = atual->num_processos;
    while (pos > 0 && atual->processos[pos - 1].prioridade > processo->prioridade) {
        atual->processos[pos] = atual->processos[pos - 1];
        pos--;
    }

    atual->processos[pos] = (ProcessoAgrupado){
        .id = processo->id,
        .prioridade = processo->prioridade,
        .tempo_inicio = processo->tempo_inicio,
    };
    strcpy(atual->processos[pos].executavel, processo->executavel);

    atual->num_processos++;
}

GerenciadorProcessos carregar_processos(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    GerenciadorProcessos gerenciador;
    inicializar_gerenciador(&gerenciador);

    Processo processo;
    while (fscanf(arquivo, "%d %s %d %d", &processo.id, processo.executavel, &processo.tempo_inicio, &processo.prioridade) == 4) {
        adicionar_processo_ao_gerenciador(&gerenciador, &processo);
        gerenciador.total_processos++;
    }

    fclose(arquivo);
    return gerenciador;
}


TempoProcessos *obter_proximo_tempo(GerenciadorProcessos *gerenciador) {
    if (gerenciador->inicio == NULL) {
        return NULL;
    }

    TempoProcessos *tempo = gerenciador->inicio;
    gerenciador->inicio = tempo->proximo;
    return tempo;
}

void adicionar_processos(SchedulerShared *scheduler, GerenciadorProcessos *gerenciador) {
    while (gerenciador->inicio != NULL) {
        TempoProcessos *tempo_atual = obter_proximo_tempo(gerenciador);

        // Aguarda até o tempo de início
        while (time(NULL) - scheduler->tempo_inicio < tempo_atual->tempo_inicio) {
            sleep(1);
        }


        // Adiciona processos deste tempo ao escalonador
        for (int i = 0; i < tempo_atual->num_processos; i++) {
            ProcessoAgrupado *processo = &tempo_atual->processos[i];
            adicionar_entrada_tabela(&scheduler->tabela, processo->id, processo->prioridade, processo->executavel);
            adicionar_fila(&scheduler->filas_prioridade[processo->prioridade], processo->id);
            printf("Processo %d (%s) adicionado à fila de prioridade %d no tempo %d.\n",
                   processo->id, processo->executavel, processo->prioridade, tempo_atual->tempo_inicio);
        }

        free(tempo_atual);
        // imprimir_tabela_processos(&scheduler->tabela);
    }
}

void imprimir_gerenciador(GerenciadorProcessos *gerenciador) {
    printf("\nGerenciador de Processos:\n");
    TempoProcessos *atual = gerenciador->inicio;

    while (atual != NULL) {
        printf("Tempo de início: %d\n", atual->tempo_inicio);
        for (int i = 0; i < atual->num_processos; i++) {
            ProcessoAgrupado *processo = &atual->processos[i];
            printf("  ID: %d, Executável: %s, Prioridade: %d\n",
                   processo->id, processo->executavel, processo->prioridade);
        }
        atual = atual->proximo;
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
               entrada->executavel,
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


