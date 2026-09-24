# Simulador de Escalonamento de Processos (MLFQ)

Repositório: https://github.com/Oliveira223/sistemas-operacionais-t1

Trabalho da disciplina de Sistemas Operacionais: um simulador de
escalonamento com 2 filas MLFQ, que interpreta processos escritos numa
linguagem assembly hipotética (baseada em acumulador) e simula a execução
deles, tick a tick (1 instrução = 1 unidade de tempo).

O enunciado completo está em `docs/TP1_20262.pdf`. Uma explicação mais detalhada
de como o projeto é organizado está em [`docs/arquitetura.md`](docs/arquitetura.md).

## Requisitos

- `g++` com suporte a C++11
- `make`

## Como compilar

Na raiz do repositório:

```bash
make
```

Gera o executável `simulador`. Para remover:

```bash
make clean
```

Para rodar os testes automatizados de cada módulo (assembler, process,
scheduler):

```bash
make test
```

## Como rodar

```bash
./simulador
```

Roda o cenário padrão (`tests/processos.txt`, os dois processos de
exemplo do enunciado). Pra rodar outro cenário, sem precisar recompilar:

```bash
./simulador caminho/para/outro_config.txt
```

O arquivo de config tem uma linha por processo, no formato
`nome arrival prioridade caminho.asm` (linhas em branco ou começando com
`#` são ignoradas). A cada UT, o simulador imprime quem rodou (Gantt
textual), o estado de cada processo, e o conteúdo da Fila 0, Fila 1 e da
lista de bloqueados; ao final, mostra o turnaround de cada processo e a
espera média na fila de prontos.

## Estrutura

```
src/    código-fonte: assembler (parser), process (PCB + step()),
        scheduler (MLFQ + tick()), main (integração/config/saída)
tests/  arquivos .asm de teste, config de exemplo, e um test_*.cpp por
        módulo (rodados via `make test`)
docs/   documentação da arquitetura e o enunciado original (TP1_20262.pdf)
```

## Testando com os casos do enunciado

`tests/p1.asm` e `tests/p2.asm` são os dois programas de exemplo do
enunciado (`tests/processos.txt` carrega os dois juntos, com arrivals 0 e 1).
A implementação segue a regra de 1 UT por instrução, inclusive saltos, e a
convenção temporal confirmada para `SYSCALL 1/2`: a chamada executa em `t`, o
processo fica bloqueado nas 3 UTs seguintes e retorna à Fila 0 em `t+4`.

Com os dois processos juntos, a execução atual produz:

```text
P1: turnaround=12 (CPU=5, bloqueio=3, espera=4)
P2: turnaround=28 (CPU=17, bloqueio=9, espera=2)
Espera media na fila de prontos: 3 UT
```

Os valores diferem do exemplo numérico do PDF, cuja linha do tempo não é
compatível com essas duas regras em todos os pontos. Os testes automatizados
seguem as regras formais adotadas pela implementação.
