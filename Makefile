# Nome do projeto
PROJECT = escalona

# Compilador e flags
CC = gcc

# Arquivos do projeto principal
SRC = main.c scheduler.c shared_memory.c process_manager.c
HEADERS = scheduler.h shared_memory.h process_manager.h
OBJS = $(SRC:.c=.o)

# Executáveis para teste
TESTS = teste10 teste20 teste30

# Regras principais
all: $(PROJECT) $(TESTS)

# Regra para o projeto principal
$(PROJECT): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROJECT) $(OBJS)

# Regra para compilar os testes
$(TESTS): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Regra para limpar os arquivos gerados
clean:
	rm -f $(PROJECT) $(OBJS) $(TESTS)

# Regras auxiliares
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean
