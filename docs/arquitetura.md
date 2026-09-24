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
  process::criarProcesso()     <- "instancia" um Programa: cada processo
      |                            tem seu próprio acc, pc, memória e nome
      v
  Scheduler + tick()           <- decide qual Process roda a cada UT,
      |                          aplicando as regras da Fila 0 / Fila 1
      v
  main.cpp                     <- laço principal: lê o config, chama
                                   tick() UT a UT, imprime estado/Gantt/
                                   filas, calcula estatísticas finais
```

Cada módulo só depende do anterior. `assembler` não sabe que `Process`
existe; `Process` não sabe que `Scheduler` existe. Isso permite testar cada
um isolado, comparando com os valores que o próprio enunciado já fornece.

## Módulos

| Módulo | Arquivo(s) | Responsabilidade |
|---|---|---|
| assembler | `src/assembler.h`, `src/assembler.cpp` | Ler um `.asm`, devolver um `Programa` pronto (instruções + variáveis), com labels já resolvidos em índices |
| process | `src/process.h`, `src/process.cpp` | Representar um processo em execução (nome, acc, pc, memória, estado, prioridade) e executar uma instrução por vez (`step()`) |
| scheduler | `src/scheduler.h`, `src/scheduler.cpp` | As duas filas do MLFQ, a lista de bloqueados, e `tick()`: decide quem roda a cada UT e devolve o que aconteceu |
| main | `src/main.cpp` | Lê o arquivo de config, admite processos no arrival certo, roda o loop de UT chamando `tick()`, imprime Gantt/estado/filas por UT e as estatísticas finais |

## Assembler

### O que ele recebe e devolve

`parse(caminho)` recebe o caminho de um arquivo `.asm` (texto) e devolve um
`Programa` - a versão já processada e pronta pra ser executada, sem mais
nenhuma referência a texto do arquivo original.

### Tipos definidos em `assembler.h`

- **`Opcode`** — enum com cada mnemônico suportado (`ADD`, `SUB`, `MULT`,
  `DIV`, `LOAD`, `STORE`, `BRANY`, `BRPOS`, `BRZERO`, `BRNEG`, `SYSCALL`).
- **`TipoOperando`** — como o operando de uma instrução deve ser lido:
  `IMEDIATO` (um número literal, ex: `#5`) ou `DIRETO` (nome de uma
  variável a buscar na memória do processo, ex: `valor`). O valor de uma
  variável não é resolvido aqui — muda durante a execução, então só o
  *nome* é guardado; quem resolve o valor de verdade é o `process`.
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

### Como `parse()` funciona (`assembler.cpp`)

1. Abre o arquivo e lê linha por linha com `getline`.
2. Cada linha passa primeiro por uma checagem de **marcador de seção**:
   `.code`, `.endcode`, `.data`, `.enddata` (via `linha.find(...)`). Achar
   um desses só atualiza em qual seção o parser está — a própria linha do
   marcador não vira instrução nem variável.
3. Dentro de `.code`: uma linha que termina em `:` é só um label (ex:
   `loop:`) — não vira `Instrucao`, só guarda num mapa (`rotulos`) o
   índice que a *próxima* instrução real vai ocupar. Toda outra linha vira
   uma `Instrucao`: o primeiro token é o opcode (traduzido via o mapa
   `opcodesPorNome`), o segundo é o operando (interpretado como imediato,
   direto, ou índice de syscall, dependendo do opcode).
4. Dentro de `.data`: cada linha é `nome valor`, direto pra
   `programa.variaveis`.
5. **Resolução de labels em 2 passadas**: como um salto pode referenciar
   um label que só aparece mais adiante no arquivo (`BRPOS loop`, com
   `loop:` abaixo), a 1ª passada (o loop acima) só coleta os labels; uma
   2ª passada, depois do arquivo inteiro lido, resolve o `alvoSalto` de
   cada instrução de salto usando o mapa já completo.

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

## `process` — detalhado

`Process` é o PCB: `nome`, `instrucoes` (cópia das do `Programa`), `acc`,
`pc`, `memoria` (cópia independente de `programa.variaveis` — um `STORE`
num processo nunca afeta outro que rode o mesmo `.asm`), `estado`
(`PRONTO`/`BLOQUEADO`/`FINALIZADO`) e `prioridade`.

`step(Process&)` executa a instrução em `instrucoes[pc]` e avança `pc`.
Um `switch` no opcode despacha pra cada categoria; saltos tomados escrevem
`pc` diretamente e retornam antes do `pc++` padrão do final da função.
`SYSCALL 0` marca `FINALIZADO`; `SYSCALL 1`/`2` marcam `BLOQUEADO` (o
countdown de 3 UT do bloqueio é responsabilidade do `scheduler`, não do
`process`).

## `scheduler` — detalhado (núcleo do trabalho)

`Scheduler` guarda três estruturas: `fila0` (deque simples, RR puro),
`fila1` (`map<prioridade, deque<processo+quantum restante>>`, RR por
prioridade) e `bloqueados` (vetor de processo+countdown). Só guarda
processos **ativos** — um processo finalizado simplesmente sai de todas as
listas, sem deixar rastro (é por isso que `main.cpp` mantém seu próprio
registro de estatísticas, ver abaixo).

- `avancarFila0()` roda 1 UT de quem está na frente da Fila 0. Se estourar
  o quantum (2 UT) sem terminar/bloquear, rebaixa pra Fila 1.
- `avancarFila1()` roda 1 UT de quem está na frente do grupo de maior
  prioridade não-vazio. Estourou quantum (4 UT) → volta pro fim do
  próprio grupo. Se simplesmente não for chamada nesse UT (porque a Fila 0
  tem prioridade), o processo continua exatamente onde estava — é assim
  que a regra de preempção ("volta pro topo, mantém tempo restante") é
  satisfeita sem precisar de nenhuma função dedicada.
- `avancarBloqueados()` decrementa o countdown de todo mundo bloqueado (I/O
  roda em paralelo à CPU, não 1 de cada vez) e devolve quem zerou pro fim
  da Fila 0.
- `tick()` avança 1 UT inteira: bloqueados sempre andam; a CPU vai pra
  Fila 0 se ela tiver alguém, senão pra Fila 1. Devolve um `ResultadoTick`
  (quem rodou, se imprimiu e com que valor, se terminou).

## `main.cpp` — detalhado

Lê o arquivo de config (`nome arrival prioridade caminho.asm` por linha,
comentários com `#`), monta a lista de processos pendentes, e roda um
loop de UT até todos terminarem. A cada UT: admite quem chega naquele
instante, chama `tick()`, imprime o resultado (Gantt textual), o estado de
cada processo (`Pronto`/`Executando`/`Bloqueado`/`Finalizado` — descoberto
por eliminação, sem precisar de registro extra pra quem já terminou) e o
conteúdo das 3 estruturas do `Scheduler`. Mantém um registro próprio
(`RegistroProcesso`) com `arrival`, instante de encerramento, tempo de CPU
e tempo de bloqueio acumulados — usado no final pra calcular turnaround
por processo e a espera média na fila de prontos.

O caminho do arquivo de config pode ser passado como argumento
(`./simulador outro.txt`); sem argumento, usa `tests/processos.txt`.

## Regras do escalonador

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

Cada módulo tem seu `test_*.cpp` próprio (`make test` compila e roda os
três em sequência): `test_assembler.cpp` e `test_process.cpp` usam
`tests/p1.asm`/`tests/p2.asm` como gabarito; `test_scheduler.cpp` cobre os
dois casos isolados mais fáceis de errar (estouro de quantum vs.
preempção) e o cenário completo P1+P2 do enunciado.

**Divergência conhecida com o PDF**: seguindo a regra escrita do
enunciado ("cada instrução leva 1 UT") à risca, incluindo os saltos
(`BRPOS`), o turnaround do P2 na nossa simulação dá 25 UT, não os 17 que o
PDF afirma — o P1 (que não tem salto no programa) bate exatamente com o
PDF (turnaround 11). O exemplo numérico do próprio PDF parece não contar
o `BRPOS` como consumindo uma UT, o que contradiz a regra que ele mesmo
escreve. Essa divergência está documentada e testada explicitamente em
`test_scheduler.cpp`.
