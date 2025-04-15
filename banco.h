#ifndef BANCO_H
#define BANCO_H

#include <pthread.h>

#define MAX_REGISTROS 100

typedef struct
{
    int id;
    char nome[50];
} Registro;

extern pthread_mutex_t mutex_banco;
void inicializar_banco();
void salvar_banco();
int inserir_registro(int id, const char *nome);
int deletar_registro(int id);
Registro *buscar_registro(int id);
#endif