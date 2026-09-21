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
