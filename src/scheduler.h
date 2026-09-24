#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include <deque>
#include <map>
#include <string>
#include <vector>

// Cada processo na Fila 1 carrega, junto, quanto do quantum de 4 UT ainda não usou nessa rodada. Isso é necessário pela regra de preempção: se a Fila 0 tomar a CPU antes do quantum estourar, o processo volta ao topo do seu grupo de prioridade com esse valor intacto, em vez de reiniciar em 4, diferente de "estourou o quantum", que reseta e vai pro fim.
struct EntradaFila1
{
    Process processo;
    int quantumRestante;
};

// Processo esperando i/o (SYSCALL 1 ou 2): countdown regressivo de 3 UT, decrementado a cada tick(). Ao chegar em 0, volta pro fim da Fila 0.
struct Bloqueado
{
    Process processo;
    int restante;
};

// O que aconteceu num UT - devolvido por tick() pro main.cpp imprimir (Gantt, "processo X imprimiu Y", etc.), sem main.cpp precisar espiar dentro das filas do Scheduler pra descobrir sozinho.
struct ResultadoTick
{
    std::string quemRodou;  // nome de quem rodou nesse UT, ou "" se CPU ociosa
    bool imprimiu;          // true se a instrução rodada foi SYSCALL 1
    int valorImpresso;      // acc no momento do print; só faz sentido se imprimiu==true
    bool terminou;          // true se o processo terminou (SYSCALL 0) nesse UT
};

struct Scheduler
{
    // Fila 0 (alta prioridade): Round Robin puro, quantum = 2 UT, FIFO. Nunca é preemptada por ninguém acima dela, então não precisa lembrar "quantum restante" entre uma execução e outra - por isso guarda só Process, sem o wrapper que a Fila 1 usa.
    std::deque<Process> fila0;

    // Fila 1 (baixa prioridade): Round Robin por prioridade estática (1-5, maior = mais prioritário), quantum = 4 UT. Chave do map é a prioridade; dentro de cada grupo, FIFO (front = próximo a rodar, back = quem chegou agora ou estourou o quantum).
    std::map<int, std::deque<EntradaFila1>> fila1;

    // Processos bloqueados esperando I/O.
    std::vector<Bloqueado> bloqueados;

    // Quantos UT o processo na FRENTE da fila0 já rodou nessa passagem. Só precisa de um contador (não um por processo, como na Fila 1) porque a Fila 0 nunca é interrompida por ninguém: uma vez que o processo da frente começa a rodar, ele roda em UTs consecutivos até terminar, bloquear ou estourar o quantum. Não existe "pausa no meio" pra precisar lembrar o valor por processo. Reseta pra 0 toda vez que a frente da fila0 muda (o processo de antes saiu por algum motivo).
    int quantumUsadoFila0 = 0;
};

// Roda 1 UT do processo na frente da fila0 (chama step() uma vez). Se ele terminar, bloquear ou estourar o quantum (2 UT), sai da fila0 pro destino certo (bloqueados, fila1, ou simplesmente removido se terminou). Devolve o que aconteceu nesse UT. Só deve ser chamada quando scheduler.fila0 não está vazia.
ResultadoTick avancarFila0(Scheduler &scheduler);

// Entrada obrigatória na Fila 0: tanto processo novo (chegou pela primeira vez) quanto retorno de I/O (saiu da lista de bloqueados) passam por aqui. Sempre entra no fim da fila0 (nunca no topo), nem direto na Fila 1.
void admitirFila0(Scheduler &scheduler, Process processo);

// Roda 1 UT do processo na frente do grupo de prioridade mais alto que tiver alguém na fila1 (map é ordenado por chave, então varre de trás pra frente = maior prioridade primeiro). Mesma ideia do avancarFila0, mas o quantum (4 UT) é contado por processo (EntradaFila1.quantumRestante), não por um contador único do Scheduler. Chamada segura mesmo com a fila1 inteira vazia: devolve um ResultadoTick "vazio" (quemRodou == "") em vez de rodar alguém.
ResultadoTick avancarFila1(Scheduler &scheduler);

// Decrementa o countdown de TODOS os processos em scheduler.bloqueados (diferente de avancarFila0/avancarFila1: I/O não disputa CPU, então todo mundo bloqueado anda 1 UT ao mesmo tempo, não só o "da frente"). Quem chega a 0 sai da lista e volta pro fim da Fila 0 (admitirFila0).
void avancarBloqueados(Scheduler &scheduler);

// Avança 1 UT inteira do sistema: bloqueados sempre andam (I/O é paralelo à CPU), e a CPU vai pra Fila 0 se ela tiver alguém, senão pra Fila 1; se as duas estiverem vazias, a CPU fica ociosa nesse UT.
ResultadoTick tick(Scheduler &scheduler);

#endif
