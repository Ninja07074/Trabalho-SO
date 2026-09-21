#include "banco.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

using namespace std;

MemoriaCompartilhada *shm = NULL;
HANDLE sem_vagas = NULL;
HANDLE sem_tarefas = NULL;
HANDLE mutex_fila = NULL;

HANDLE sem_resp = NULL;
HANDLE sem_resp_lida = NULL;

int conectar() {
  HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHM_NOME);
  if (!hMap)
    return 0;
  shm = (MemoriaCompartilhada *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0,
                                              sizeof(MemoriaCompartilhada));

  sem_vagas = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEM_VAGAS);
  sem_tarefas = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEM_TAREFAS);
  mutex_fila = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_FILA);
  sem_resp = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEM_RESP);
  sem_resp_lida = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEM_RESP_LIDA);

  return (shm && sem_vagas && sem_tarefas && mutex_fila && sem_resp &&
          sem_resp_lida);
}

void enviar_requisicao(Requisicao req) {
  // produtor: Coloca o pedido na Fila da Memória Compartilhada
  WaitForSingleObject(sem_vagas, INFINITE);
  WaitForSingleObject(mutex_fila, INFINITE);

  shm->fila_req[shm->fim_req] = req;
  shm->fim_req = (shm->fim_req + 1) % MAX_FILA;

  ReleaseMutex(mutex_fila);
  ReleaseSemaphore(sem_tarefas, 1,
                   NULL); // avisa a thread do servidor que chegou pedido

  // se a requisição for para fechar o servidor, não espera resposta
  if (req.op == OP_SAIR)
    return;

  // espera o servidor processar e colocar a resposta
  WaitForSingleObject(sem_resp, INFINITE);
  Resposta resp = shm->resp;
  ReleaseSemaphore(sem_resp_lida, 1,
                   NULL); // avisa o servidor que já leu a resposta, liberando
                          // pra outro cliente

  cout << "\n>>> RESPOSTA DO SERVIDOR <<<" << endl;
  cout << "Status: " << (resp.status == 1 ? "[SUCESSO]" : "[ERRO]") << endl;
  cout << "Mensagem: " << resp.msg << endl;
  if (req.op == OP_SELECT && resp.status == 1) {
    cout << "Dados: ID = " << resp.id << " | Nome = " << resp.nome << endl;
  }
  cout << "------------------------------\n" << endl;
}

int main() {
  if (!conectar()) {
    cout << "[ERRO] Nao foi possivel conectar. O servidor esta rodando?" << endl;
    return 1;
  }

  cout << "[CLIENTE] Conectado com sucesso!" << endl;

  int opcao = -1;
  while (opcao != 0) {
    cout << "==============================" << endl;
    cout << "       CLIENTE DO BANCO       " << endl;
    cout << "==============================" << endl;
    cout << " [1] Inserir (INSERT)" << endl;
    cout << " [2] Consultar (SELECT)" << endl;
    cout << " [3] Atualizar (UPDATE)" << endl;
    cout << " [4] Deletar (DELETE)" << endl;
    cout << " [9] Desligar Servidor (SAIR)" << endl;
    cout << " [0] Fechar Cliente" << endl;
    cout << "==============================" << endl;
    cout << "Escolha uma opcao: ";

    if (!(cin >> opcao))
      break;

    Requisicao req;
    memset(&req, 0, sizeof(Requisicao));

    if (opcao == 1) {
      req.op = OP_INSERT;
      cout << "Digite o ID: ";
      cin >> req.id;
      cout << "Digite o Nome: ";
      cin >> req.nome;
      enviar_requisicao(req);
    } else if (opcao == 2) {
      req.op = OP_SELECT;
      cout << "Digite o ID: ";
      cin >> req.id;
      enviar_requisicao(req);
    } else if (opcao == 3) {
      req.op = OP_UPDATE;
      cout << "Digite o ID: ";
      cin >> req.id;
      cout << "Digite o Novo Nome: ";
      cin >> req.nome;
      enviar_requisicao(req);
    } else if (opcao == 4) {
      req.op = OP_DELETE;
      cout << "Digite o ID: ";
      cin >> req.id;
      enviar_requisicao(req);
    } else if (opcao == 9) {
      req.op = OP_SAIR;
      // envia N vezes para fechar as 4 threads no servidor
      for (int i = 0; i < 4; i++) {
        enviar_requisicao(req);
      }
      cout << "[CLIENTE] Servidor desligado. Fechando cliente..." << endl;
      break;
    } else if (opcao == 0) {
      cout << "[CLIENTE] Fechando cliente (Servidor continua rodando)..." << endl;
    } else {
      cout << "[CLIENTE] Opcao invalida!" << endl;
    }
  }

  return 0;
}
