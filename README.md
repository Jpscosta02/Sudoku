# Projeto Sudoku (Cliente/Servidor) — Compilação e Execução

Sistema **cliente/servidor** para jogar Sudoku via **TCP**:
- **Individual** (cada cliente joga o seu Sudoku)
- **Competição** (2 equipas, tabuleiro partilhado por equipa + ranking final)

> Nota: a pasta do projeto chama-se `Projeto/` (P maiúsculo).

## Requisitos

- Sistema compatível POSIX (Linux/WSL recomendado)
- `gcc` e `make`
- Suporte a *pthread* (o `makefile` compila com `-pthread`)
- 2+ terminais (servidor + clientes)

## Ficheiros importantes

- `makefile` — compila `servidorApp` e `clienteApp`
- `server.conf` — configuração do servidor (porta, nº clientes, ficheiro de jogos)
- `client*.conf` — configuração de cada cliente (IP, porta, ID, equipa)
- `jogos.txt` — lista de jogos no formato `id,puzzle,solucao` (81 chars para puzzle/solução)
- `logs/` — logs gerados automaticamente (criado se não existir)

## Compilar (recomendado)

Na raiz do projeto, faça **sempre** limpeza antes de compilar:

```bash
cd Projeto
make clean
make
```

Binários gerados:
- `./servidorApp`
- `./clienteApp`

## Executar

### 1) Servidor
No diretório `Projeto/`:

```bash
./servidorApp server.conf
```

- Parar: `Ctrl+C`
- Porta ocupada: altere `PORTA` em `server.conf` e nos `client*.conf`.

### 2) Cliente(s)
Em outro terminal, ainda dentro de `Projeto/`:

```bash
./clienteApp client1.conf
```

O cliente apresenta um menu:
- `1) Jogar sozinho`
- `2) Jogar em competição (equipas)`
- `3) Sair`

### Nota sobre caminhos

- O servidor lê o ficheiro configurado em `server.conf` (por omissão `jogos.txt`).
- O cliente também lê `jogos.txt` localmente (para validação/apoio no menu).

Por isso, a forma mais simples é **executar sempre a partir de `Projeto/`** (para que `jogos.txt` seja encontrado).

## Modo competição (equipas)

- Existem **2 equipas** (`EQUIPA: 1` ou `EQUIPA: 2` nos `client*.conf`).
- O servidor sincroniza o arranque com uma barreira: o modo competição só avança quando entram **`MAX_CLIENTES` clientes**.
- Se não entrarem todos dentro de ~30s, o servidor devolve erro de “tempo limite na barreira”.

Exemplo com a configuração incluída (`MAX_CLIENTES: 4`):

1. Arrancar o servidor: `./servidorApp server.conf`
2. Em 4 terminais, arrancar:
   - `./clienteApp client1.conf` (equipa 1)
   - `./clienteApp client2.conf` (equipa 1)
   - `./clienteApp client3.conf` (equipa 2)
   - `./clienteApp client4.conf` (equipa 2)
3. Em cada cliente, escolher `2) Jogar em competição (equipas)`

No fim, quando todas as equipas terminarem um Sudoku correto, o servidor envia o **ranking final**.

## Logs

Os logs são escritos em:
- `logs/servidor.log`
- `logs/cliente_<ID_CLIENTE>.log` (ex.: `logs/cliente_Cliente1.log`)

Se quiser reiniciar o histórico, apague a pasta `logs/` ou execute `make clean` (que também a remove).
