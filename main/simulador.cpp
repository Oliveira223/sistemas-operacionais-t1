#include "simula.h"
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

static string estadoParaString(Estado estado) {
    switch (estado) {
        case Estado::NOVO: return "NOVO";
        case Estado::PRONTO: return "PRONTO";
        case Estado::EXECUTANDO: return "EXECUTANDO";
        case Estado::BLOQUEADO: return "BLOQUEADO";
        case Estado::FINALIZADO: return "FINALIZADO";
    }
    return "?";
}

void Simula::adicionarProcesso(const Processo& processo) {
    processos.push_back(processo);
}

void Simula::processaChegadas() {
    for (int i = 0; i < static_cast<int>(processos.size()); ++i) {
        Processo& p = processos[i];
        if (p.estado == Estado::NOVO && p.arrivalTime == tempo) {
            escalanador.adicionarFila0(i, processos);
        }
    }
}

void Simula::processaDesbloqueios() {
    for (int i = 0; i < static_cast<int>(processos.size()); ++i) {
        Processo& p = processos[i];
        if (p.estado == Estado::BLOQUEADO && p.desbloqueio == tempo) {
            p.desbloqueio = -1;
            escalanador.adicionarFila0(i, processos);
        }
    }
}

void Simula::verificaPreempcao() {
    if (cpu == -1) {
        return;
    }

    Processo& atual = processos[cpu];
    if (atual.filaAtual == Fila::FILA_1 && escalanador.fila0TemProcesso()) {
        escalanador.devolveInicioFila1(cpu, processos);
        cpu = -1;
    }
}

void Simula::despachar() {
    if (cpu != -1) {
        return;
    }

    cpu = escalanador.escolherProximo();
    if (cpu != -1) {
        processos[cpu].estado = Estado::EXECUTANDO;
    }
}

void Simula::contabilizaEspera() {
    // Somente processos no estado PRONTO acumulam tempo de espera.
    for (Processo& p : processos) {
        if (p.estado == Estado::PRONTO) {
            ++p.espera;
        }
    }
}

void Simula::executaCPU() {
    if (cpu == -1) {
        return;
    }

    Processo& p = processos[cpu];
    const ResultExec resultado = exec.executar(p);
    --p.rest;

    // Finalizacao e bloqueio sao tratados antes do fim de quantum.
    if (resultado == ResultExec::FINALIZADO) {
        p.estado = Estado::FINALIZADO;
        p.filaAtual = Fila::NENHUMA;
        p.tempoFinalizacao = tempo + 1;
        cpu = -1;
        return;
    }

    if (resultado == ResultExec::BLOQUEADO) {
        p.estado = Estado::BLOQUEADO;
        p.filaAtual = Fila::NENHUMA;

        // A SYSCALL ocupa a UT atual. Depois dela, o processo permanece
        // bloqueado por tres UTs completas e retorna no inicio da quarta.
        p.desbloqueio = tempo + 4;
        cpu = -1;
        return;
    }

    if (p.rest == 0) {
        const int indice = cpu;
        if (p.filaAtual == Fila::FILA_0) {
            escalanador.rebaixaFila(indice, processos);
        }
        else {
            escalanador.fimQuantFila1(indice, processos);
        }
        cpu = -1;
    }
}

bool Simula::todosFinalizados() const {
    if (processos.empty()) {
        return true;
    }

    for (const Processo& p : processos) {
        if (p.estado != Estado::FINALIZADO) {
            return false;
        }
    }
    return true;
}

void Simula::imprimirEstadoAtual() const {
    cout << "\n===== UT " << tempo << " =====\n";

    if (cpu == -1) {
        cout << "CPU: IDLE\n";
    }
    else {
        const Processo& p = processos[cpu];
        cout << "CPU: " << p.nome
                  << " (q=" << p.rest << ")\n";
    }

    cout << "F0: [";
    const deque<int>& f0 = escalanador.pegaFila0();
    for (size_t i = 0; i < f0.size(); ++i) {
        const Processo& p = processos[f0[i]];
        cout << p.nome << "(q=" << p.rest << ')';
        if (i + 1 < f0.size()) {
            cout << ", ";
        }
    }
    cout << "]\n";

    cout << "F1:\n";
    const array<deque<int>, 5>& f1 = escalanador.pegaFila1();
    for (int prioridade = 5; prioridade >= 1; --prioridade) {
        const deque<int>& grupo = f1[prioridade - 1];
        cout << "  prioridade " << prioridade << ": [";
        for (size_t i = 0; i < grupo.size(); ++i) {
            const Processo& p = processos[grupo[i]];
            cout << p.nome << "(q=" << p.rest << ')';
            if (i + 1 < grupo.size()) {
                cout << ", ";
            }
        }
        cout << "]\n";
    }

    cout << "Bloqueados: [";
    bool primeiro = true;
    for (const Processo& p : processos) {
        if (p.estado == Estado::BLOQUEADO) {
            if (!primeiro) {
                cout << ", ";
            }
            cout << p.nome;
            primeiro = false;
        }
    }
    cout << "]\n";

    cout << "Estados:\n";
    for (const Processo& p : processos) {
        // NOVO e apenas um estado interno antes do arrival time.
        if (p.estado != Estado::NOVO) {
            cout << "  " << p.nome << ": " << estadoParaString(p.estado) << '\n';
        }
    }
}

void Simula::imprimirGantt() const {
    cout << "\n===== GANTT =====\n";
    for (size_t t = 0; t < gantt.size(); ++t) {
        cout << "t=" << t << " -> ";
        if (gantt[t] == -1) {
            cout << "IDLE";
        }
        else {
            cout << processos[gantt[t]].nome;
        }
        cout << '\n';
    }
}

void Simula::imprimirEstatisticas() const {
    cout << "\n===== ESTATISTICAS =====\n";

    double somaEspera = 0.0;
    for (const Processo& p : processos) {
        const int turnaround = p.tempoFinalizacao - p.arrivalTime;
        somaEspera += p.espera;

        cout << p.nome
                  << ": turnaround=" << turnaround << " UT"
                  << ", espera=" << p.espera << " UT\n";
    }

    if (!processos.empty()) {
        cout << fixed << setprecision(2)
                  << "Tempo medio de espera: "
                  << (somaEspera / static_cast<double>(processos.size()))
                  << " UT\n";
    }
}

void Simula::executar() {
    while (!todosFinalizados()) {
        // Eventos do inicio da UT sao tratados antes de escolher quem executa.
        processaChegadas();
        processaDesbloqueios();
        verificaPreempcao();
        despachar();

        imprimirEstadoAtual();
        gantt.push_back(cpu);
        contabilizaEspera();
        executaCPU();

        ++tempo;
    }

    imprimirGantt();
    imprimirEstatisticas();
}
