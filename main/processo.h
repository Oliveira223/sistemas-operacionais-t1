#ifndef PROCESSO_H
#define PROCESSO_H

///////////////////////////////////////////

#include <string>
#include <unordered_map>
#include "programa.h"

enum class Estado{
    NOVO,
    PRONTO,
    EXECUTANDO,
    BLOQUEADO,
    FINALIZADO
};

enum class Fila{
    NENHUMA,
    FILA_0,
    FILA_1
};

struct Processo{
    string nome;
    int arrivalTime = 0;
    int prioridade = 1;

    Estado estado = Estado::NOVO;
    Fila filaAtual = Fila::NENHUMA;

    int programCounter = 0;
    int acc = 0;
    int rest = 0;
    int desbloqueio = -1;
    int espera = 0;
    int tempoFinalizacao = -1;
    Programa program;
    unordered_map<string, int> dados;
};

///////////////////////////////////////////

#endif