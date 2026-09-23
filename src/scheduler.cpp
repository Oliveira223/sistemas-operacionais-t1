#include "scheduler.h"

// Monta o ResultadoTick a partir de quem acabou de rodar e da instrução que
// foi executada (capturada ANTES do step(), já que depois dele processo.pc
// já avançou e não dá mais pra saber qual instrução foi essa).
static ResultadoTick montarResultado(const Process &processo, const Instrucao &instrucaoExecutada)
{
    ResultadoTick resultado;
    resultado.quemRodou = processo.nome;
    resultado.imprimiu = instrucaoExecutada.opcode == Opcode::SYSCALL && instrucaoExecutada.indiceSyscall == 1;
    resultado.valorImpresso = processo.acc;
    resultado.terminou = processo.estado == Estado::FINALIZADO;
    return resultado;
}

ResultadoTick avancarFila0(Scheduler &scheduler)
{
    Process &processo = scheduler.fila0.front();

    Instrucao instrucaoExecutada = processo.instrucoes[processo.pc];
    step(processo);
    scheduler.quantumUsadoFila0++;

    ResultadoTick resultado = montarResultado(processo, instrucaoExecutada);

    // 3 situações tiram o processo da frente da fila0 (as outras duas não
    // têm nada a decidir; só a 3ª calcula pra onde ele vai):

    // 1. Terminou (SYSCALL 0) -> sai do sistema, não vai pra lugar nenhum
    if (processo.estado == Estado::FINALIZADO)
    {
        scheduler.fila0.pop_front();
        scheduler.quantumUsadoFila0 = 0;
    }

    // 2. Bloqueou (SYSCALL 1/2) -> vai pra lista de bloqueados esperar o I/O
    else if (processo.estado == Estado::BLOQUEADO)
    {
        scheduler.bloqueados.push_back({processo, 3});
        scheduler.fila0.pop_front();
        scheduler.quantumUsadoFila0 = 0;
    }

    // 3. Estourou o quantum (2 UT) sem terminar nem bloquear -> rebaixa pra
    // Fila 1, entra no fim do grupo da sua prioridade com quantum cheio (4 UT)
    else if (scheduler.quantumUsadoFila0 == 2)
    {
        scheduler.fila1[processo.prioridade].push_back({processo, 4});
        scheduler.fila0.pop_front();
        scheduler.quantumUsadoFila0 = 0;
    }
    // senão: continua na frente da fila0, quantumUsadoFila0 guarda a contagem

    return resultado;
}

void admitirFila0(Scheduler &scheduler, Process processo)
{
    // processo novo já chega PRONTO (criarProcesso); retorno de I/O ainda
    // está com estado == BLOQUEADO (ninguém mais reseta isso) — sem essa
    // linha, ele ficaria marcado como bloqueado pra sempre, mesmo já
    // rodando de novo na Fila 0.
    processo.estado = Estado::PRONTO;
    scheduler.fila0.push_back(processo);
}

ResultadoTick avancarFila1(Scheduler &scheduler)
{
    // fila1 é um map ordenado por prioridade crescente; rbegin()/rend()
    // percorre de trás pra frente, ou seja, do maior grupo pro menor.
    for (auto grupo = scheduler.fila1.rbegin(); grupo != scheduler.fila1.rend(); ++grupo)
    {
        std::deque<EntradaFila1> &fila = grupo->second;

        if (fila.empty())
        {
            continue; // esse grupo de prioridade não tem ninguém, tenta o próximo
        }

        EntradaFila1 &entrada = fila.front();

        Instrucao instrucaoExecutada = entrada.processo.instrucoes[entrada.processo.pc];
        step(entrada.processo);
        entrada.quantumRestante--;

        ResultadoTick resultado = montarResultado(entrada.processo, instrucaoExecutada);

        if (entrada.processo.estado == Estado::FINALIZADO)
        {
            fila.pop_front();
        }
        else if (entrada.processo.estado == Estado::BLOQUEADO)
        {
            scheduler.bloqueados.push_back({entrada.processo, 3});
            fila.pop_front();
        }
        else if (entrada.quantumRestante == 0)
        {
            // estourou o quantum sem terminar nem bloquear -> fim do mesmo
            // grupo, com quantum cheio de novo (4 UT)
            EntradaFila1 copia = entrada;
            copia.quantumRestante = 4;
            fila.pop_front();
            fila.push_back(copia);
        }
        // senão: continua no topo do grupo, quantumRestante guarda a contagem

        return resultado; // achou o grupo certo e já rodou o UT — não olha os outros grupos
    }

    return ResultadoTick{}; // nenhum grupo tinha alguém -> CPU ociosa
}

void avancarBloqueados(Scheduler &scheduler)
{
    for (size_t i = 0; i < scheduler.bloqueados.size();)
    {
        scheduler.bloqueados[i].restante--;

        if (scheduler.bloqueados[i].restante == 0)
        {
            admitirFila0(scheduler, scheduler.bloqueados[i].processo);
            scheduler.bloqueados.erase(scheduler.bloqueados.begin() + i);
        }
        else
        {
            i++;
        }
    }
}

ResultadoTick tick(Scheduler &scheduler)
{
    avancarBloqueados(scheduler); // I/O anda em paralelo à CPU, todo UT

    if (!scheduler.fila0.empty())
    {
        return avancarFila0(scheduler);
    }

    return avancarFila1(scheduler); // se também vazia, devolve resultado ocioso
}
