#include <stdio.h>
#include <string.h>

#define PIPE_NAME "pipe_bd"

int main() {
    FILE *pipe = fopen(PIPE_NAME, "w");
    if (!pipe) {
        perror("Erro ao abrir pipe");
        return 1;
    }

    char comando[100];
    while (1) {
        printf("Digite uma requisição (INSERT, DELETE, SELECT, UPDATE) ou 'exit':\n");
        fgets(comando, sizeof(comando), stdin);
        if (strncmp(comando, "exit", 4) == 0) break;
        fprintf(pipe, "%s", comando);
        fflush(pipe);
    }

    fclose(pipe);
    return 0;
}