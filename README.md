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

## Como rodar

```bash
./simulador
```

**Status atual:** o programa ainda não lê um arquivo de configuração de
processos — por enquanto, `main.cpp` chama o parser (`assembler`) direto
num arquivo de teste (`tests/p1.asm`), só pra validar a leitura do
assembly. O escalonador (`scheduler`) e a execução de processos (`process`)
ainda não foram implementados; ver [`docs/arquitetura.md`](docs/arquitetura.md)
para o que já existe e o que falta.

## Estrutura

```
src/    código-fonte (assembler, e futuramente process/scheduler/main)
tests/  arquivos .asm de teste, incluindo os dois exemplos do enunciado
docs/   documentação da arquitetura e do que cada módulo faz
```

## Testando com os casos do enunciado

`tests/p1.asm` e `tests/p2.asm` são os dois programas de exemplo do
enunciado, cujo resultado esperado (turnaround, tempo médio de espera,
timeline completa) já está calculado no próprio PDF. Servem como
referência: o resultado do simulador completo deve bater exatamente com o
que o enunciado descreve para esses dois processos rodando juntos.
