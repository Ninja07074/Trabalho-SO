#ifndef BANCO_H
#define BANCO_H

#include <windows.h>

// Nomes para a Memória Compartilhada e Semáforos no Windows
#define SHM_NOME       "Local\\BancoMemoriaM1"
#define SEM_VAGAS      "Local\\BancoSemVagas"
#define SEM_TAREFAS    "Local\\BancoSemTarefas"
#define MUTEX_FILA     "Local\\BancoMutexFila"

#define SEM_RESP       "Local\\BancoSemResp"
#define MUTEX_RESP     "Local\\BancoMutexResp"
#define SEM_RESP_LIDA  "Local\\BancoSemRespLida"

#define ARQUIVO_BANCO  "banco.txt"

// Operações do Banco
#define OP_SAIR   0
#define OP_INSERT 1
#define OP_SELECT 2
#define OP_UPDATE 3
#define OP_DELETE 4

#define MAX_REGISTROS 100
#define MAX_FILA 10

// Estrutura interna do banco (linha de dados)
typedef struct {
    int id;
    char nome[50];
    int ocupado;
} Registro;

// O que o Cliente envia para o Servidor
typedef struct {
    int op;
    int id;
    char nome[50];
} Requisicao;

// O que o Servidor devolve para o Cliente
typedef struct {
    int status; // 1 = OK, 0 = Erro
    char msg[100];
    int id;
    char nome[50];
} Resposta;

// Essa estrutura inteira fica na Memória Compartilhada do Windows
typedef struct {
    // 1. A Fila de Requisições (Padrão Produtor-Consumidor)
    Requisicao fila_req[MAX_FILA];
    int inicio_req;
    int fim_req;

    // 2. A Resposta (simplificada, protegida por mutex)
    Resposta resp;
} MemoriaCompartilhada;

#endif
