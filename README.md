# SO Project

## Como rodar

``` shell
$ make
$ ./escalona $NUMBER_OF_CORES $QUANTUM $INPUT_FILE
```

## Funcionamento

### Estruturas principais
- GerenciadorProcessos - guarda os processos lidos do arquivo de entrada
- EntradaTabela
- TabelaProcessos
- SchedulerShared - objeto que é compartilhado entre o gerenciador de processos e o escalonador

### Gerenciador de processos
- Criado no main.c a partir de um fork.
- Responsável por adicionar os processos nas devidas filas no tempo correto.
- Os processos são adicionados a partir da struct GerenciadorProcessos que organiza-os de acordo com o tempo e ordena-os de acordo com a prioridade.

### Escalonador
- Existe um loop principal que só termina quando todos os processos forem executados
- Dividido em duas partes dentro do loop principal: Executar e Checar
- Executar: Loop nas filas de prioridade e cria um novo processo para executar o programa definido
- Checar: Loop nos processos que estão sendo executados para checar se foi finalizado ou se deveria acontecer o quantum.
- Checagem do término do processo é feita a partir do waitpid com a opção WNOHANG para obter o status do processo. Caso o status seja 0 quer dizer que o processo ainda está rodando, caso contrário ele finalizou.
- O quantum do processo é simulado a partir do envio do sinal SIGSTOP. É usa o sinal SIGCONT para retomar a execução do processo posteriormente.
