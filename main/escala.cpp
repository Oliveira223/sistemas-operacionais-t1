#include "escala.h"
#include <stdexcept>

using namespace std;

static int grupoPrioridade(const Processo& processo){
    if(processo.prioridade < 1 || processo.prioridade > 5){
        throw runtime_error("Prioridade fora do intervalo no processo " + processo.nome);
    }
    return processo.prioridade - 1;
}

void Escala::adicionarFila0(int indice, vector<Processo>& processos){
    Processo& p = processos.at(indice);
    p.estado = Estado::PRONTO;
    p.filaAtual = Fila::FILA_0;
    p.rest = 2;
    fila0.push_back(indice);
}

void Escala::rebaixaFila(int indice, vector<Processo>& processos){
    Processo& p = processos.at(indice);
    p.estado = Estado::PRONTO;
    p.filaAtual = Fila::FILA_1;
    p.rest = 4;
    fila1[grupoPrioridade(p)].push_back(indice);
}

void Escala::fimQuantFila1(int indice, std::vector<Processo>& processos) {
    Processo& p = processos.at(indice);
    p.estado = Estado::PRONTO;
    p.filaAtual = Fila::FILA_1;
    p.rest = 4;
    fila1[grupoPrioridade(p)].push_back(indice);
}

void Escala::devolveInicioFila1(int indice, std::vector<Processo>& processos) {
    Processo& p = processos.at(indice);
    p.estado = Estado::PRONTO;
    p.filaAtual = Fila::FILA_1;

    // Preempcao pela F0 nao reinicia o quantum da F1 e devolve o processo
    // ao inicio do seu grupo de prioridade.
    fila1[grupoPrioridade(p)].push_front(indice);
}

int Escala::escolherProximo() {
    // A Fila 0 sempre tem prioridade global sobre a Fila 1.
    if (!fila0.empty()) {
        const int indice = fila0.front();
        fila0.pop_front();
        return indice;
    }

    // Na Fila 1, a maior prioridade estatica e escolhida primeiro.
    for (int prioridade = 5; prioridade >= 1; --prioridade) {
        deque<int>& grupo = fila1[prioridade - 1];
        if (!grupo.empty()) {
            const int indice = grupo.front();
            grupo.pop_front();
            return indice;
        }
    }

    return -1;
}

bool Escala::fila0TemProcesso() const{
    return !fila0.empty();
}

const deque<int>& Escala::pegaFila0() const{
    return fila0;
}

const array<deque<int>, 5>& Escala::pegaFila1() const{
    return fila1;
}