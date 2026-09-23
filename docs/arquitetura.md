# Arquitetura do simulador

Simulador de escalonamento de processos com 2 filas MLFQ (Multi-Level
Feedback Queue), para a disciplina de Sistemas Operacionais. Lê processos
descritos numa linguagem assembly hipotética (baseada em acumulador) e
simula a execução deles, um "tick" (1 instrução = 1 unidade de tempo) por
vez, decidindo qual processo roda a cada momento segundo as regras do
escalonador.

## Pipeline geral

```
arquivo .asm (texto)
      |
      v
  assembler::parse()          <- lê o arquivo, entende .code/.data,
      |                          resolve labels, devolve Programa
      v
  struct Programa              <- instruções + variáveis iniciais
      |
      v
  struct Process (Fase 2)      <- "instancia" um Programa: cada processo
      |                            tem seu próprio acc, pc e memória
      v
  Scheduler (Fase 3)           <- decide qual Process roda a cada tick,
      |                            aplicando as regras da Fila 0 / Fila 1
      v
  main.cpp (Fase 4)            <- laço principal: chama o scheduler tick
                                   a tick, imprime estado/Gantt/filas
```

Cada módulo só depende do anterior. `assembler` não sabe que `Process`
existe; `Process` não sabe que `Scheduler` existe. Isso permite testar cada
um isolado, comparando com os valores que o próprio enunciado já fornece.

## Módulos

| Módulo | Arquivo(s) | Status | Responsabilidade |
|---|---|---|---|
| assembler | `src/assembler.h`, `src/assembler.cpp` | Em andamento | Ler um `.asm`, devolver um `Programa` pronto (instruções + variáveis) |
| process | `src/process.h`, `src/process.cpp` | Não iniciado | Representar um processo em execução (acc, pc, memória, estado) e executar uma instrução por vez |
| scheduler | `src/scheduler.h`, `src/scheduler.cpp` | Não iniciado | As duas filas do MLFQ, a lista de bloqueados, e a decisão de quem roda a cada tick |
| main | `src/main.cpp` | Provisório | Por enquanto só chama `parse()` pra testar o assembler manualmente |

## `assembler` — detalhado

### O que ele recebe e devolve

`parse(caminho)` recebe o caminho de um arquivo `.asm` (texto) e devolve um
`Programa` — a versão já processada e pronta pra ser executada, sem mais
nenhuma referência a texto do arquivo original.

### Tipos definidos em `assembler.h`

- **`Opcode`** — enum com cada mnemônico suportado (`ADD`, `SUB`, `MULT`,
  `DIV`, `LOAD`, `STORE`, `BRANY`, `BRPOS`, `BRZERO`, `BRNEG`, `SYSCALL`).
- **`TipoOperando`** — como o operando de uma instrução deve ser lido:
  `IMEDIATO` (um número literal, ex: `#5`) ou `DIRETO` (nome de uma
  variável a buscar na memória do processo, ex: `valor`). O valor de uma
  variável não é resolvido aqui — muda durante a execução, então só o
  *nome* é guardado; quem resolve o valor de verdade é o `process` (Fase 2).
- **`Instrucao`** — uma linha do `.code`, já interpretada:
  - `opcode` / `tipoOperando` — o que é a instrução e como ler seu operando.
  - `valorImediato` — usado quando `tipoOperando == IMEDIATO`.
  - `nomeVariavel` — usado quando `tipoOperando == DIRETO`.
  - `indiceSyscall` — usado só por `SYSCALL` (0 = encerrar, 1 = imprimir, 2 = ler).
  - `alvoSalto` — usado só pelas instruções de salto (`BRANY`/`BRPOS`/
    `BRZERO`/`BRNEG`): o índice, na lista de instruções, pra onde pular.
    O parser já resolve o label de texto (`loop:`) pra esse número —
    quem executa não lida mais com nomes de label, só com posições.
- **`Programa`** — o resultado final: `instrucoes` (lista, na ordem do
  `.code`) e `variaveis` (mapa nome → valor inicial, do `.data`).

### Como `parse()` funciona hoje (`assembler.cpp`)

1. Abre o arquivo com `std::ifstream` e lê linha por linha com `getline`.
2. Cada linha passa primeiro por uma checagem de **marcador de seção**:
   `.code`, `.endcode`, `.data`, `.enddata` (via `linha.find(...)`). Achar
   um desses só atualiza em qual seção o parser está (`Secao::CODE`,
   `Secao::DATA` ou `Secao::NENHUMA`) — a própria linha do marcador não
   vira instrução nem variável.
3. Linhas em branco são ignoradas.
4. **Ainda não implementado**: transformar o conteúdo de cada linha (fora
   dos marcadores) numa `Instrucao` (dentro de `.code`) ou num par
   nome/valor em `programa.variaveis` (dentro de `.data`). Hoje essas
   linhas só são impressas na tela pra conferência visual — é o próximo
   passo.
5. Comentários no `.asm` (depois de `#`) ainda não têm tratamento
   dedicado — a estratégia planejada é ler cada linha em "tokens"
   separados por espaço e usar só os dois primeiros (opcode + operando),
   ignorando o resto da linha sem precisar identificar o que é comentário.

### Linguagem assembly reconhecida

| Categoria | Mnemônico | Função |
|---|---|---|
| Aritmético | `ADD`/`SUB`/`MULT`/`DIV op1` | `acc = acc (op) op1` |
| Memória | `LOAD op1` | `acc = (op1)` |
| Memória | `STORE op1` | `(op1) = acc` — só endereçamento direto |
| Salto | `BRANY label` | `pc = label` |
| Salto | `BRPOS label` | se `acc > 0`, `pc = label` |
| Salto | `BRZERO label` | se `acc == 0`, `pc = label` |
| Salto | `BRNEG label` | se `acc < 0`, `pc = label` |
| Sistema | `SYSCALL index` | `0` = encerra, `1`/`2` = imprime/lê (bloqueia 3 UT) |

Operando imediato usa `#N`; operando direto usa o nome da variável.
`STORE` só aceita direto. Cada instrução/variável ocupa 1 posição de
memória (não há gerenciador de memória real neste trabalho).

## Regras do escalonador (referência para a Fase 3)

- **Fila 0** (alta prioridade): Round Robin puro, quantum = 2 UT, FIFO.
  Todo processo novo ou retorno de I/O entra no fim dela. Estourou o
  quantum sem terminar/pedir I/O → desce pra Fila 1.
- **Fila 1** (baixa prioridade): Round Robin por prioridade estática
  (1–5, maior número = mais prioritário), quantum = 4 UT, empate por
  FIFO. Estourou quantum → volta ao **fim** do próprio grupo de
  prioridade. Foi **preemptado** (a Fila 0 recebeu alguém enquanto este
  processo rodava) → volta ao **topo** do próprio grupo, mantendo o
  tempo de execução já usado.
- Fila 0 sempre tem prioridade sobre a Fila 1: se há qualquer processo
  pronto na Fila 0, é ele quem roda.
- Troca de contexto tem custo zero.

## Testes

`tests/p1.asm` e `tests/p2.asm` são os dois programas de exemplo do
enunciado, com resultado já calculado manualmente por ele (turnaround e
tempo médio de espera). Servem de "gabarito": cada módulo só é considerado
pronto quando o resultado dele bate exatamente com o que o enunciado prevê
para esses dois programas.
