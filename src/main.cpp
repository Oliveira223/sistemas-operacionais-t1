#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "scheduler.h"

using namespace std;

// Processo ainda não admitido no sistema, esperando seu arrival.
struct Pendente
{
    Process processo;
    int arrival;
};

// Lê o arquivo de config, formato "nome arrival prioridade caminho.asm".
// Linhas em branco ou começando com # são ignoradas.
vector<Pendente> lerConfig(string caminho)
{
    vector<Pendente> pendentes;
    ifstream arquivo(caminho);

    string linha;
    while (getline(arquivo, linha))
    {
        if (linha.empty() || linha[0] == '#')
        {
            continue;
        }

        istringstream streamDaLinha(linha);
        string nome;
        int arrival;
        int prioridade;
        string caminhoAsm;
        streamDaLinha >> nome >> arrival >> prioridade >> caminhoAsm;

        Programa programa = parse(caminhoAsm);
        pendentes.push_back({criarProcesso(programa, nome, prioridade), arrival});
    }

    return pendentes;
}

// Imprime o conteúdo das estruturas do scheduler após a execução da UT.
void imprimirFilas(const Scheduler &scheduler)
{
    cout << "  Fila0: ";
    for (const Process &p : scheduler.fila0)
    {
        cout << p.nome << " ";
    }
    cout << "\n";

    cout << "  Fila1: ";
    for (auto grupo = scheduler.fila1.rbegin(); grupo != scheduler.fila1.rend(); ++grupo)
    {
        for (const EntradaFila1 &entrada : grupo->second)
        {
            cout << entrada.processo.nome << "(p" << grupo->first << ") ";
        }
    }
    cout << "\n";

    cout << "  Bloqueados: ";
    for (const Bloqueado &b : scheduler.bloqueados)
    {
        // Mostra só quem está bloqueado. O contador é detalhe interno do scheduler.
        cout << b.processo.nome << " ";
    }
    cout << "\n";
}

// Estado do processo APÓS a execução da UT.
// Quem executou DURANTE a UT aparece separadamente na linha "CPU=...".
string descobrirEstado(const string &nome, const Scheduler &scheduler)
{
    for (const Process &p : scheduler.fila0)
    {
        if (p.nome == nome)
        {
            return "Pronto";
        }
    }

    for (const auto &grupo : scheduler.fila1)
    {
        for (const EntradaFila1 &entrada : grupo.second)
        {
            if (entrada.processo.nome == nome)
            {
                return "Pronto";
            }
        }
    }

    for (const Bloqueado &b : scheduler.bloqueados)
    {
        if (b.processo.nome == nome)
        {
            return "Bloqueado";
        }
    }

    return "Finalizado";
}

// Confere se um processo continua na lista de bloqueados após o avanço do tick.
bool estaBloqueado(const string &nome, const Scheduler &scheduler)
{
    for (const Bloqueado &b : scheduler.bloqueados)
    {
        if (b.processo.nome == nome)
        {
            return true;
        }
    }
    return false;
}

// Estatísticas mantidas fora do Scheduler, pois processos finalizados saem das filas.
struct RegistroProcesso
{
    string nome;
    int arrival;
    int encerramento = -1;
    int tempoCpu = 0;
    int tempoBloqueio = 0;
};

bool aindaNaoChegou(const string &nome, const vector<Pendente> &pendentes)
{
    for (const Pendente &p : pendentes)
    {
        if (p.processo.nome == nome)
        {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[])
{
    string caminhoConfig = (argc > 1) ? argv[1] : "tests/processos.txt";
    vector<Pendente> pendentes = lerConfig(caminhoConfig);

    vector<string> todosNomes;
    map<string, RegistroProcesso> registros;
    for (const Pendente &p : pendentes)
    {
        todosNomes.push_back(p.processo.nome);

        RegistroProcesso registro;
        registro.nome = p.processo.nome;
        registro.arrival = p.arrival;
        registros[p.processo.nome] = registro;
    }

    int totalProcessos = pendentes.size();

    Scheduler scheduler;
    int finalizados = 0;

    // Cada repetição representa uma UT completa do sistema.
    for (int ut = 0; finalizados < totalProcessos; ut++)
    {
        // Admite antes do tick para quem chega em t já disputar a CPU em t.
        for (size_t i = 0; i < pendentes.size();)
        {
            if (pendentes[i].arrival == ut)
            {
                admitirFila0(scheduler, pendentes[i].processo);
                pendentes.erase(pendentes.begin() + i);
            }
            else
            {
                i++;
            }
        }

        vector<string> bloqueadosAntes;
        for (const Bloqueado &b : scheduler.bloqueados)
        {
            bloqueadosAntes.push_back(b.processo.nome);
        }

        ResultadoTick r = tick(scheduler);

        for (const string &nome : bloqueadosAntes)
        {
            if (nome != r.quemRodou && estaBloqueado(nome, scheduler))
            {
                registros[nome].tempoBloqueio++;
            }
        }

        if (!r.quemRodou.empty())
        {
            registros[r.quemRodou].tempoCpu++;
        }

        if (r.terminou)
        {
            registros[r.quemRodou].encerramento = ut + 1;
        }

        // Quem ocupou a CPU DURANTE a UT.
        cout << "UT" << ut << ": ";
        if (!r.quemRodou.empty())
        {
            cout << "CPU=" << r.quemRodou << " (Executando)";
            if (r.imprimiu)
            {
                cout << " | imprime " << r.valorImpresso;
            }
            if (r.terminou)
            {
                cout << " | finaliza ao fim da UT";
            }
        }
        else
        {
            cout << "CPU ociosa";
        }
        cout << "\n";

        // Situação resultante APÓS a execução da UT.
        cout << "  Estados ao final da UT: ";
        for (const string &nome : todosNomes)
        {
            if (aindaNaoChegou(nome, pendentes))
            {
                cout << nome << "=NaoChegou ";
            }
            else
            {
                cout << nome << "=" << descobrirEstado(nome, scheduler) << " ";
            }
        }
        cout << "\n";

        imprimirFilas(scheduler);

        if (r.terminou)
        {
            finalizados++;
        }
    }

    cout << "\n=== Estatisticas finais ===\n";
    int somaEspera = 0;

    for (const string &nome : todosNomes)
    {
        RegistroProcesso &registro = registros[nome];

        int turnaround = registro.encerramento - registro.arrival;
        int espera = turnaround - registro.tempoCpu - registro.tempoBloqueio;
        somaEspera += espera;

        cout << nome << ": turnaround=" << turnaround
             << " (CPU=" << registro.tempoCpu
             << ", bloqueio=" << registro.tempoBloqueio
             << ", espera=" << espera << ")\n";
    }

    double esperaMedia = (double)somaEspera / totalProcessos;
    cout << "Espera media na fila de prontos: " << esperaMedia << " UT\n";

    return 0;
}
