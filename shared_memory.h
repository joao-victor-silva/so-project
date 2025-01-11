#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include "scheduler.h"
#include <stdbool.h>

// Função para inicializar a memória compartilhada
SchedulerShared *inicializar_memoria_compartilhada(int numero_de_cores, int quantum, int processos, int *shm_id);

// Função para destruir a memória compartilhada
void destruir_memoria_compartilhada(int shm_id, SchedulerShared *scheduler);

bool adicionar_fila(Fila *fila, int id);
bool remover_fila(Fila *fila, int *id);

void adicionar_entrada_tabela(TabelaProcessos *tabela, int id, int , const char *executavel);
EntradaTabela *obter_entrada_tabela(TabelaProcessos *tabela, int id);

#endif // SHARED_MEMORY_H
