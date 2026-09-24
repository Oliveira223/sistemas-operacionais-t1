# Simulador de Escalonamento de Processos (MLFQ)

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
enunciado (`tests/processos.txt` já carrega os dois juntos, arrival 0 e 1
como o PDF pede). O turnaround do P1 bate exatamente com o que o PDF
calcula (11 UT). O do P2 diverge (25 UT na nossa simulação, contra 17 no
PDF) — seguimos à risca a regra escrita do enunciado de que toda
instrução, incluindo os saltos (`BRPOS`), consome 1 UT; o exemplo
numérico do próprio PDF parece não contar o salto do laço do P2 dessa
forma. Essa divergência está documentada e testada em
`tests/test_scheduler.cpp` e detalhada em `docs/arquitetura.md`.
