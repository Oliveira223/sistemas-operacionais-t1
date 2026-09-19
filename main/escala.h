#ifndef ESCALA_H
#define ESCALA_H

///////////////////////////////////////////
#include <array>
#include <deque>
#include <vector>
#include "processo.h"

using namespace std;

class Escala {
private:
    deque<int> fila0;
    array<deque<int>, 5> fila1;

public: 
    void adicionarFila0(int indicie, vector<Processo>& processos);
    void rebaixaFila(int indicie, vector<Processo>& processos);
    void fimQuantFila1(int indicie, vector<Processo>& processos);
    void devolveInicioFila1(int indicie, vector<Processo>& processos);

    int escolherProximo();
    bool fila0TemProcesso() const;

    const deque<int>& pegaFila0() const;
    const array<deque<int>, 5>& pegaFila1() const;
};

///////////////////////////////////////////

#endif