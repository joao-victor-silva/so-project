#include <stdio.h>

int main() {
    long i;
    for (i = 0; i < 99000000000; i++); // Simula a execução por 20 segundos
    return 0;
}