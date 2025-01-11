#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include "scheduler.h"
#include "shared_memory.h"
#include "process_manager.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <numero_de_cores> <quantum> <arquivo_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int numero_de_cores = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    const char *arquivo_entrada = argv[3];

    GerenciadorProcessos gerenciador_processos = carregar_processos(arquivo_entrada);
    imprimir_gerenciador(&gerenciador_processos);

    int shm_id;
    SchedulerShared *scheduler = inicializar_memoria_compartilhada(numero_de_cores, quantum, gerenciador_processos.total_processos, &shm_id);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Erro ao criar processo filho");
        destruir_memoria_compartilhada(shm_id, scheduler);
        return EXIT_FAILURE;
    } else if (pid == 0) {
        adicionar_processos(scheduler, &gerenciador_processos);
        imprimir_tabela_processos(&scheduler->tabela);
        exit(0);
    } else {
        executar_scheduler(scheduler);
        printf("Ordem de execução dos processos:\n");
        for (int i = 0; i < scheduler->ordem_index; i++) {
            printf("Processo %d\n", scheduler->ordem[i]);
        }
        wait(NULL);
        destruir_memoria_compartilhada(shm_id, scheduler);
    }

    return EXIT_SUCCESS;
}
