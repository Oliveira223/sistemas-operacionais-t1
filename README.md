# Simulador de Escalonamento de Processos

Este repositório contém a implementação de um simulador de sistemas operacionais.

## Como Compilar e Executar no Windows

Há diversas formas de compilar o simulador, dependendo do ambiente que você está utilizando.

### 1. Usando Scripts do PowerShell (Recomendado no Windows)

Para facilitar a compilação nativa no Windows (sem precisar de ferramentas extras como `make`), foram criados dois scripts.

**Compilar:**
Abra o PowerShell na pasta do projeto e execute:
```powershell
.\compilar.ps1
```
Isso vai gerar o executável `simulador.exe`.

**Executar testes:**
Para rodar automaticamente todos os arquivos `processos.txt` dentro da pasta `casos/`:
```powershell
.\executar_testes.ps1
```

### 2. Usando CMake (Ideal para VS Code / CLion)

O projeto possui um arquivo `CMakeLists.txt` configurado.
- **VS Code:** Instale a extensão "CMake Tools". O editor vai reconhecer o projeto automaticamente. Basta clicar em *Build* (na barra inferior) e depois em *Run* ou *Debug*.
- **CLion / Visual Studio:** Basta abrir a pasta do projeto. A IDE fará a leitura do CMake automaticamente e liberará o botão de compilar/rodar.

### 3. Usando Make (Linux, WSL ou MinGW)

Se você possui o comando `make` instalado no seu ambiente, basta rodar na raiz do projeto:

```bash
make
```

Para limpar os arquivos gerados:
```bash
make clean
```

---

## Estrutura de Arquivos de Teste

Os arquivos de configuração do simulador (geralmente chamados de `processos.txt`) seguem o formato:
`[nome_processo] [tempo_chegada] [prioridade] [caminho_arquivo_asm]`

**Exemplo:**
```text
P1 0 3 casos/prioridade/p1.asm
P2 0 5 casos/prioridade/p2.asm
```

*Nota:* Certifique-se de que o caminho apontado no arquivo `.txt` exista em relação ao diretório onde você está executando o simulador.
