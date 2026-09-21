#include "banco.h"
#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

using namespace std;

#define N_THREADS 4

// banco de Dados na memória
Registro tabela[MAX_REGISTROS];

// mutex do Pthreads para proteger a Seção Crítica
pthread_mutex_t mutex_sc = PTHREAD_MUTEX_INITIALIZER;

// variáveis de IPC
MemoriaCompartilhada *shm = NULL;
HANDLE sem_vagas = NULL;
HANDLE sem_tarefas = NULL;
HANDLE mutex_fila = NULL;

HANDLE sem_resp = NULL;
HANDLE mutex_resp = NULL;
HANDLE sem_resp_lida = NULL;

// funções para salvar e carregar do TXT
void salvar_banco() {
  FILE *f = fopen(ARQUIVO_BANCO, "w");
  if (!f)
    return;
  for (int i = 0; i < MAX_REGISTROS; i++) {
    if (tabela[i].ocupado) {
      fprintf(f, "%d;%s\n", tabela[i].id, tabela[i].nome);
    }
  }
  fclose(f);
}

void carregar_banco() {
  for (int i = 0; i < MAX_REGISTROS; i++)
    tabela[i].ocupado = 0;
  FILE *f = fopen(ARQUIVO_BANCO, "r");
  if (!f)
    return;
  int id;
  char nome[50];
  int count = 0;
  // tenta ler os dados, ignorando formatações vazias
  while (fscanf(f, "%d;%49[^\n]\n", &id, nome) == 2 ||
         fscanf(f, "%d;%s\n", &id, nome) == 2) {
    if (count < MAX_REGISTROS) {
      tabela[count].id = id;
      strcpy(tabela[count].nome, nome);
      tabela[count].ocupado = 1;
      count++;
    }
  }
  fclose(f);
}

void imprimir_tabela() {
  cout << "\n=== BANCO DE DADOS ===" << endl;
  int vazia = 1;
  for (int i = 0; i < MAX_REGISTROS; i++) {
    if (tabela[i].ocupado) {
      cout << "ID: " << tabela[i].id << " | Nome: " << tabela[i].nome << endl;
      vazia = 0;
    }
  }
  if (vazia)
    cout << "(Tabela Vazia)" << endl;
  cout << "======================\n" << endl;
}

// a função que as threads do pool vão executar (Consumidor)
void *trabalhador(void *arg) {
  long id_thread = (long)arg;

  while (1) {
    // espera ter tarefa na fila
    WaitForSingleObject(sem_tarefas, INFINITE);

    // protege a fila para retirar a requisicao
    WaitForSingleObject(mutex_fila, INFINITE);
    Requisicao req = shm->fila_req[shm->inicio_req];
    shm->inicio_req = (shm->inicio_req + 1) % MAX_FILA;
    ReleaseMutex(mutex_fila);

    // libera uma vaga na fila
    ReleaseSemaphore(sem_vagas, 1, NULL);

    if (req.op == OP_SAIR) {
      break; // se o cliente mandou sair, encerra a thread
    }

    cout << "[Thread " << id_thread << "] Recebeu requisicao OP: " << req.op << ", ID: " << req.id << endl;

    Resposta resp;
    memset(&resp, 0, sizeof(Resposta));
    resp.id = req.id;

    //
    // inicio da seção crítica (protegida por Pthread)
    //
    pthread_mutex_lock(&mutex_sc);
    cout << "[Thread " << id_thread << "] entrou na seção crítica." << endl;

    Sleep(300); // simula o atraso de HD pedido pelo professor para forçar o
                // paralelismo

    if (req.op == OP_INSERT) {
      int achou = 0, livre = -1;
      for (int i = 0; i < MAX_REGISTROS; i++) {
        if (tabela[i].ocupado && tabela[i].id == req.id)
          achou = 1;
        if (!tabela[i].ocupado && livre == -1)
          livre = i;
      }
      if (achou) {
        resp.status = 0;
        strcpy(resp.msg, "Erro: ID ja existe");
      } else if (livre != -1) {
        tabela[livre].id = req.id;
        strcpy(tabela[livre].nome, req.nome);
        tabela[livre].ocupado = 1;
        salvar_banco();
        resp.status = 1;
        strcpy(resp.msg, "Inserido com sucesso");
      } else {
        resp.status = 0;
        strcpy(resp.msg, "Erro: Banco cheio");
      }
    } else if (req.op == OP_SELECT) {
      int achou = 0;
      for (int i = 0; i < MAX_REGISTROS; i++) {
        if (tabela[i].ocupado && tabela[i].id == req.id) {
          resp.status = 1;
          strcpy(resp.nome, tabela[i].nome);
          strcpy(resp.msg, "Encontrado");
          achou = 1;
          break;
        }
      }
      if (!achou) {
        resp.status = 0;
        strcpy(resp.msg, "Erro: ID nao encontrado");
      }
    } else if (req.op == OP_DELETE) {
      int achou = 0;
      for (int i = 0; i < MAX_REGISTROS; i++) {
        if (tabela[i].ocupado && tabela[i].id == req.id) {
          tabela[i].ocupado = 0;
          salvar_banco();
          resp.status = 1;
          strcpy(resp.msg, "Removido com sucesso");
          achou = 1;
          break;
        }
      }
      if (!achou) {
        resp.status = 0;
        strcpy(resp.msg, "Erro: ID nao encontrado");
      }
    } else if (req.op == OP_UPDATE) {
      int achou = 0;
      for (int i = 0; i < MAX_REGISTROS; i++) {
        if (tabela[i].ocupado && tabela[i].id == req.id) {
          strcpy(tabela[i].nome, req.nome);
          salvar_banco();
          resp.status = 1;
          strcpy(resp.msg, "Atualizado com sucesso");
          achou = 1;
          break;
        }
      }
      if (!achou) {
        resp.status = 0;
        strcpy(resp.msg, "Erro: ID nao encontrado");
      }
    }

    imprimir_tabela();

    cout << "[Thread " << id_thread << "] Saiu da Secao Critica." << endl;
    pthread_mutex_unlock(&mutex_sc);
    //
    // FIM DA SEÇÃO CRÍTICA
    //

    // Envia a resposta para o Cliente
    WaitForSingleObject(mutex_resp,
                        INFINITE); // uma thread responde por vez
    shm->resp = resp;
    ReleaseSemaphore(sem_resp, 1, NULL); // avisa o cliente que a resposta tá lá

    WaitForSingleObject(sem_resp_lida,
                        INFINITE); // espera o cliente avisar que leu
    ReleaseMutex(
        mutex_resp); // libera o canal de respostas para a próxima thread
  }
  return NULL;
}

int main() {
  cout << "   SERVIDOR RODANDO " << endl << endl;

  carregar_banco();

  // cria a Memória Compartilhada
  HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                   0, sizeof(MemoriaCompartilhada), SHM_NOME);
  shm = (MemoriaCompartilhada *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0,
                                              sizeof(MemoriaCompartilhada));

  // zera a fila no inicio
  shm->inicio_req = 0;
  shm->fim_req = 0;

  // inicia os Semáforos e Mutexes do Windows
  sem_vagas = CreateSemaphoreA(NULL, MAX_FILA, MAX_FILA, SEM_VAGAS);
  sem_tarefas = CreateSemaphoreA(NULL, 0, MAX_FILA, SEM_TAREFAS);
  mutex_fila = CreateMutexA(NULL, FALSE, MUTEX_FILA);

  sem_resp = CreateSemaphoreA(NULL, 0, 1, SEM_RESP);
  mutex_resp = CreateMutexA(NULL, FALSE, MUTEX_RESP);
  sem_resp_lida = CreateSemaphoreA(NULL, 0, 1, SEM_RESP_LIDA);

  // cria o Pool de Threads (Exigência do Professor)
  pthread_t threads[N_THREADS];
  for (long i = 0; i < N_THREADS; i++) {
    pthread_create(&threads[i], NULL, trabalhador, (void *)(i + 1));
  }

  cout << "[SERVIDOR] Pool com " << N_THREADS << " threads criado com sucesso!" << endl;
  cout << "[SERVIDOR] Aguardando clientes enviarem tarefas...\n" << endl;

  for (int i = 0; i < N_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  UnmapViewOfFile(shm);
  CloseHandle(hMap);
  return 0;
}
