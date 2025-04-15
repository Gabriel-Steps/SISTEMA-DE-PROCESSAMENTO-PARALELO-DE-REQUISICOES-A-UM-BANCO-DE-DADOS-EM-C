#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "banco.h"

#define PIPE_NAME "pipe_bd"

pthread_mutex_t mutex_banco;
Registro banco[MAX_REGISTROS];
int num_registros = 0;

// PROCESSAMENTO DE THREAD
void *processar_requisicao(void *arg)
{
    char *requisicao = (char *)arg;
    char operacao[10], nome[50] = "";
    int id;

    // Extrai operação, id e nome (se houver)
    sscanf(requisicao, "%s %d %49[^\n]", operacao, &id, nome);

    pthread_mutex_lock(&mutex_banco);

    if (strcmp(operacao, "INSERT") == 0)
    {
        inserir_registro(id, nome);
        printf("Registro inserido: %d - %s\n", id, nome);
    }
    else if (strcmp(operacao, "DELETE") == 0)
    {
        deletar_registro(id);
        printf("Registro deletado com id: %d\n", id);
    }
    else if (strcmp(operacao, "SELECT") == 0)
    {
        Registro *r = buscar_registro(id);
        if (r)
        {
            printf("Registro encontrado: %d - %s\n", r->id, r->nome);
        }
        else
        {
            printf("Registro com id %d não encontrado.\n", id);
        }
    }
    else if (strcmp(operacao, "UPDATE") == 0)
    {
        atualizar_registro(id, nome);
    }
    else
    {
        printf("Operação desconhecida: %s\n", operacao);
    }

    pthread_mutex_unlock(&mutex_banco);
    free(requisicao);
    return NULL;
}

// MAIN
int main()
{
    pthread_mutex_init(&mutex_banco, NULL);
    inicializar_banco();

    // Criar pipe se não existir
    if (access(PIPE_NAME, F_OK) == -1)
    {
        if (mkfifo(PIPE_NAME, 0666) != 0)
        {
            perror("Erro ao criar pipe");
            return 1;
        }
    }

    FILE *pipe = fopen(PIPE_NAME, "r");
    if (!pipe)
    {
        perror("Erro ao abrir pipe");
        return 1;
    }

    char buffer[100];
    while (fgets(buffer, sizeof(buffer), pipe))
    {
        pthread_t thread;
        char *requisicao = strdup(buffer);
        pthread_create(&thread, NULL, processar_requisicao, requisicao);
        pthread_detach(thread);
    }

    fclose(pipe);
    pthread_mutex_destroy(&mutex_banco);
    return 0;
}

// FUNÇÕES DO BANCO
void inicializar_banco()
{
    // Tentativa de carregar do banco.json
    FILE *f = fopen("banco.json", "r");
    if (!f)
        return;

    char linha[1024];
    fread(linha, sizeof(char), sizeof(linha), f);
    fclose(f);

    char *ptr = strtok(linha, "{},:\"]\"[]");
    while (ptr != NULL)
    {
        if (strcmp(ptr, "\"id\"") == 0)
        {
            ptr = strtok(NULL, "{},:\"]\"[]");
            int id = atoi(ptr);
            ptr = strtok(NULL, "{},:\"]\"[]"); // nome
            ptr = strtok(NULL, "{},:\"]\"[]");
            char nome[50];
            strncpy(nome, ptr, 50);
            inserir_registro(id, nome);
        }
        ptr = strtok(NULL, "{},:\"]\"[]");
    }
}

void salvar_banco()
{
    FILE *f = fopen("banco.json", "w");
    fprintf(f, "[");
    for (int i = 0; i < num_registros; i++)
    {
        fprintf(f, "{\"id\":%d,\"nome\":\"%s\"}%s", banco[i].id, banco[i].nome, (i < num_registros - 1) ? "," : "");
    }
    fprintf(f, "]");
    fclose(f);
}

int inserir_registro(int id, const char *nome)
{
    if (num_registros >= MAX_REGISTROS)
        return -1;
    banco[num_registros].id = id;
    strncpy(banco[num_registros].nome, nome, 50);
    num_registros++;
    salvar_banco();
    return 0;
}

int atualizar_registro(int id, const char *novo_nome)
{
    for (int i = 0; i < num_registros; i++)
    {
        if (banco[i].id == id)
        {
            strncpy(banco[i].nome, novo_nome, 50);
            salvar_banco();
            printf("Registro atualizado: %d - %s\n", id, novo_nome);
            return 0;
        }
    }
    printf("Registro com id %d não encontrado para UPDATE.\n", id);
    return -1;
}

int deletar_registro(int id)
{
    for (int i = 0; i < num_registros; i++)
    {
        if (banco[i].id == id)
        {
            banco[i] = banco[num_registros - 1];
            num_registros--;
            salvar_banco();
            return 0;
        }
    }
    return -1;
}

Registro *buscar_registro(int id)
{
    for (int i = 0; i < num_registros; i++)
    {
        if (banco[i].id == id)
            return &banco[i];
    }
    return NULL;
}
