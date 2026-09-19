#ifndef SIMULA_H
#define SIMULA_H

///////////////////////////////////////////

#include <vector>
#include "escala.h"
#include "executor.h"
#include "processo.h"

using namespace std;

class Simula{
private:
vector<Processo> processos;
Escala escalanador;
Executor exec;

int tempo = 0;
int cpu = -1;
vector<int> gantt;

void processaChegadas();
void processaDesbloqueios();
void verificaPreempcao();
void despachar();
void contabilizaEspera();
void executaCPU();

bool todosFinalizados() const;

void imprimirEstadoAtual() const;
void imprimirGantt() const;
void imprimirEstatisticas() const;

public:
    void adicionarProcesso(const Processo& processo);
    void executar();

};

///////////////////////////////////////////

#endif