# Sistema Banco Cliente/Servidor

Este projeto implementa um sistema simulado de banco de dados baseado na arquitetura Cliente/Servidor, utilizando múltiplos processos, Threads, e Comunicação Entre Processos (IPC) através de Memória Compartilhada no Windows.

O projeto foi desenvolvido em linguagem C/C++ como avaliação da disciplina de Sistemas Operacionais.

## 🛠️ Tecnologias e Conceitos Utilizados
- **Processos Separados:** O Cliente e o Servidor rodam em binários (processos) distintos.
- **Pthreads (Pool de Threads):** O Servidor utiliza múltiplas threads para atender as requisições em paralelo.
- **IPC (Memória Compartilhada):** A comunicação entre Cliente e Servidor ocorre de forma real, sem uso de variáveis globais simples ou arquivos intermediários de polling.
- **Sincronização (Mutex e Semáforos):** Proteção da Seção Crítica e sincronização da fila de requisições garantida pelo uso de Semáforos e Mutex (da API do Windows e Pthreads).

## ⚙️ Pré-requisitos para Compilação
Como o código faz uso da API nativa do Windows (`<windows.h>`), ele **deve ser compilado e executado em ambiente Windows**. 

Você precisará ter instalado:
- Compilador **G++** (MinGW / MSYS2)
- Ferramenta **Make** (Opcional, caso queira compilar via `Makefile`)

## 🚀 Como Compilar
Se você possuir o `make` configurado, basta rodar o comando abaixo no terminal, na pasta raiz do projeto:
```cmd
make
```

Caso queira compilar manualmente via terminal, execute os comandos do `g++`:
```cmd
g++ -Wall -O2 servidor.c -o servidor.exe -pthread
g++ -Wall -O2 cliente.c -o cliente.exe
```

## ▶️ Como Executar
O sistema requer que o Servidor e o Cliente estejam rodando ao mesmo tempo.

1. **Inicie o Servidor:** 
   Abra um terminal na pasta do projeto e rode o executável do servidor:
   ```cmd
   .\servidor.exe
   ```
   *O servidor ficará rodando em "standby" aguardando tarefas.*

2. **Inicie o Cliente:** 
   Abra um **SEGUNDO terminal** e rode o executável do cliente:
   ```cmd
   .\cliente.exe
   ```

3. **Interação:**
   No terminal do cliente, você verá um menu interativo. Basta escolher as opções (Inserir, Consultar, Atualizar, Deletar) digitando o número correspondente. Acompanhe no terminal do Servidor as threads trabalhando em tempo real!
