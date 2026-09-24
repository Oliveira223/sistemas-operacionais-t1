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

// Lê o arquivo de config, formato "nome arrival prioridade caminho.asm". Linhas em branco ou começando com # são ignoradas (comentário) - lê linha por linha (em vez de >> direto do arquivo) exatamente por causa disso: precisa decidir "pular ou processar" antes de tentar extrair os 4 campos, senão uma linha de comentário quebraria a extração e pararia a leitura do arquivo inteiro ali (igual ao que já fazemos no assembler.cpp pra pular linha de label).
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

// Imprime o conteúdo das 3 estruturas do scheduler nesse instante. fila1 é varrida com rbegin()/rend() (maior prioridade primeiro), igual ao avancarFila1, assim a ordem impressa já reflete a ordem de despacho.
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
        cout << b.processo.nome << "(" << b.restante << "UT) ";
    }
    cout << "\n";
}

// Descobre o estado atual de um processo, dado seu nome. Se ele não está rodando agora nem em nenhuma das 3 listas do scheduler, só sobra uma possibilidade: já terminou (processos finalizados somem do Scheduler, não tem outro lugar onde poderiam estar). Por isso não precisa de nenhum registro extra só pra saber quem já terminou.
string descobrirEstado(const string &nome, const Scheduler &scheduler, const string &quemRodou)
{
    if (nome == quemRodou)
    {
        return "Executando";
    }

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
// Isso evita contar como bloqueio a UT em que o countdown zera e o processo retorna à Fila 0.
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

// Estatísticas que o Scheduler não guarda (ele esquece processos assim que eles saem do sistema): arrival de cada um, quando encerrou, e quanto tempo total ele passou rodando/bloqueado ao longo de TODA a simulação, acumulado UT a UT, porque tick() só sabe sobre 1 UT.
struct RegistroProcesso
{
    string nome;
    int arrival;
    int encerramento = -1; // -1 = ainda não terminou
    int tempoCpu = 0;
    int tempoBloqueio = 0;
};

// Confere se um nome ainda está entre os pendentes (ainda não chegou).
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

// Uso: ./simulador [caminho/do/config.txt]. Sem argumento, usa tests/processos.txt (cenário P1+P2 do enunciado), mas qualquer arquivo no mesmo formato pode ser passado na hora, sem precisar recompilar (importante pra rodar outros casos na apresentação).
int main(int argc, char *argv[])
{
    string caminhoConfig = (argc > 1) ? argv[1] : "tests/processos.txt";
    vector<Pendente> pendentes = lerConfig(caminhoConfig);

    // Guardado ANTES do loop, com os nomes de TODOS os processos: pendentes vai encolhendo conforme são admitidos, então não dá mais pra usar pendentes.size() (nem pendentes.at(i).processo.nome) depois de um tempo pra saber quem existe no total.
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

    // Roda até todo mundo terminar, nem um UT a mais nem a menos.
    for (int ut = 0; finalizados < totalProcessos; ut++)
    {
        // Admite quem chega nesse UT ANTES de chamar tick(): assim, quem acabou de chegar já disputa a CPU nesse mesmo UT (mesma lógica da preempção imediata, tick() sempre reavalia a Fila 0 do zero). Índice+erase é o mesmo padrão do avancarBloqueados: precisa reprocessar o índice que "tomou o lugar" de quem foi removido, por isso só incrementa i no else.
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

        // Foto de quem está bloqueado ANTES do tick(): depois da chamada alguém pode já ter saído dali (countdown chegou a 0), e nesse caso ele ainda gastou 1 UT de bloqueio nesse UT que passou.
        vector<string> bloqueadosAntes;
        for (const Bloqueado &b : scheduler.bloqueados)
        {
            bloqueadosAntes.push_back(b.processo.nome);
        }

        ResultadoTick r = tick(scheduler);

        for (const string &nome : bloqueadosAntes)
        {
            // Conta como bloqueio apenas se, depois de avancarBloqueados(), o processo ainda estiver bloqueado.
            // Se o countdown zerou neste tick, ele já voltou à Fila 0 e esta UT não é mais de bloqueio.
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
            registros[r.quemRodou].encerramento = ut + 1; // "t" = fim do UT
        }

        cout << "UT" << ut << ": ";
        if (!r.quemRodou.empty())
        {
            cout << r.quemRodou;
            if (r.imprimiu) cout << " imprime " << r.valorImpresso;
            if (r.terminou) cout << " [FINALIZADO]";
        }
        else
        {
            cout << "(ocioso)";
        }
        cout << "\n";

        cout << "  Estados: ";
        for (const string &nome : todosNomes)
        {
            if (aindaNaoChegou(nome, pendentes))
            {
                cout << nome << "=NaoChegou ";
            }
            else
            {
                cout << nome << "=" << descobrirEstado(nome, scheduler, r.quemRodou) << " ";
            }
        }
        cout << "\n";

        imprimirFilas(scheduler);

        if (r.terminou) finalizados++;
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
