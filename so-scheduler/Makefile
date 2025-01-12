# Nome do projeto
PROJECT = escalona

# Compilador e flags
CC = gcc

# Arquivos do projeto principal
SRC = main.c 
OBJS = $(SRC:.c=.o)

# Executáveis para teste
TESTS = teste10 teste20 teste30

# Regras principais
all: $(PROJECT) $(TESTS)

# Regra para o projeto principal
$(PROJECT): $(OBJS)
	$(CC) $(CFLAGS) -o src/$(PROJECT) $(OBJS)

# Regra para compilar os testes
$(TESTS): %: %.c
	$(CC) $(CFLAGS) -o $@ src/$<

# Regra para limpar os arquivos gerados
clean:
	rm -f $(PROJECT) $(OBJS) $(TESTS)

# Regras auxiliares
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean
