#include "scheduler.h"
#include <cassert>
#include <iostream>

using namespace std;

Instrucao fazerAdd1()
{
    Instrucao instrucao;
    instrucao.opcode = Opcode::ADD;
    instrucao.tipoOperando = TipoOperando::IMEDIATO;
    instrucao.valorImediato = 1;
    return instrucao;
}

Instrucao fazerSyscall(int indice)
{
    Instrucao instrucao;
    instrucao.opcode = Opcode::SYSCALL;
    instrucao.indiceSyscall = indice;
    return instrucao;
}

// Isolado: estourou o quantum (4 ADD #1 seguidos, sem terminar/bloquear) -> volta pro FIM do mesmo grupo de prioridade, com quantum resetado pra 4. Não pode ser confundido com preempção (testarPreempcao abaixo).
void testarEstourouQuantumFila1()
{
    Scheduler scheduler;
    Programa programa;
    for (int i = 0; i < 4; i++) programa.instrucoes.push_back(fazerAdd1());

    scheduler.fila1[3].push_back({criarProcesso(programa, "A", 3), 4});

    tick(scheduler);
    tick(scheduler);
    tick(scheduler);
    tick(scheduler); // 4a chamada -> estoura o quantum

    assert(scheduler.fila1[3].size() == 1);
    assert(scheduler.fila1[3].front().quantumRestante == 4); // resetou
    assert(scheduler.fila1[3].front().processo.acc == 4);
    assert(scheduler.fila1[3].front().processo.pc == 4);
    cout << "testarEstourouQuantumFila1: ok\n";
}

// Isolado: processo A no meio do quantum da Fila 1 é preemptado pela Fila 0 populando. Enquanto isso, B "chega" no mesmo grupo de prioridade. A deve retomar do TOPO do grupo (antes de B), com quantumRestante e pc preservados, não reseta como no caso de estouro acima.
void testarPreempcaoFila1()
{
    Scheduler scheduler;
    Programa programaLongo;
    for (int i = 0; i < 6; i++) programaLongo.instrucoes.push_back(fazerAdd1());

    scheduler.fila1[3].push_back({criarProcesso(programaLongo, "A", 3), 4});

    tick(scheduler); // A roda, quantumRestante 4 -> 3
    tick(scheduler); // A roda, quantumRestante 3 -> 2
    assert(scheduler.fila1[3].front().quantumRestante == 2);

    // B chega no mesmo grupo enquanto A está pausado
    scheduler.fila1[3].push_back({criarProcesso(programaLongo, "B", 3), 4});

    // Fila 0 recebe um processo novo -> preempção imediata
    Programa programaCurto;
    programaCurto.instrucoes.push_back(fazerSyscall(0));
    admitirFila0(scheduler, criarProcesso(programaCurto, "C", 1));

    ResultadoTick r = tick(scheduler);
    assert(r.quemRodou == "C"); // fila0 tomou a CPU, nem A nem B tocados
    assert(scheduler.fila1[3].front().processo.nome == "A");
    assert(scheduler.fila1[3].front().quantumRestante == 2); // intacto

    // fila0 vazia de novo -> quem retoma tem que ser A (topo), não B
    ResultadoTick r2 = tick(scheduler);
    assert(r2.quemRodou == "A");
    assert(scheduler.fila1[3].front().quantumRestante == 1); // continuou de 2
    cout << "testarPreempcaoFila1: ok\n";
}

// Integração: replica o cenário P1+P2 do enunciado (TP1_20262.pdf, seção 5), arrival P1=0/prio3, P2=1/prio5.
//
// IMPORTANTE: o PDF afirma turnaround P2=17 (finaliza em t=18), mas isso só bate se o salto BRPOS não consumir 1 UT, o que contradiz a regra escrita do próprio PDF ("cada instrução leva 1 UT"), que aplicamos aqui à risca (igual ao step() do process.cpp). Com BRPOS custando 1 UT como qualquer instrução, P2 termina em t=26 (turnaround 25), não t=18. P1 (sem saltos no programa) bate exatamente com o PDF (turnaround 11), só P2 diverge, e é o único dos dois com laço/BRPOS. Decisão registrada em NOTES/perguntas.md (seção Scheduler): seguimos a regra escrita, não o exemplo numérico do PDF, até confirmar com o professor.
void testarCenarioP1P2DoEnunciado()
{
    Programa p1prog = parse("tests/p1.asm");
    Programa p2prog = parse("tests/p2.asm");

    Scheduler scheduler;
    int finalizacaoP1 = -1, finalizacaoP2 = -1;

    for (int ut = 0; ut < 30; ut++)
    {
        if (ut == 0) admitirFila0(scheduler, criarProcesso(p1prog, "P1", 3));
        if (ut == 1) admitirFila0(scheduler, criarProcesso(p2prog, "P2", 5));

        ResultadoTick r = tick(scheduler);

        // Prints na ordem certa, com o valor certo (confirmado por execução real antes de escrever este teste, não calculado à mão)
        if (ut == 5) { assert(r.quemRodou == "P2"); assert(r.imprimiu); assert(r.valorImpresso == 2); }
        if (ut == 7) { assert(r.quemRodou == "P1"); assert(r.imprimiu); assert(r.valorImpresso == 15); }
        if (ut == 13) { assert(r.quemRodou == "P2"); assert(r.imprimiu); assert(r.valorImpresso == 1); }
        if (ut == 20) { assert(r.quemRodou == "P2"); assert(r.imprimiu); assert(r.valorImpresso == 0); }

        if (r.terminou && r.quemRodou == "P1") finalizacaoP1 = ut;
        if (r.terminou && r.quemRodou == "P2") finalizacaoP2 = ut;
    }

    int turnaroundP1 = finalizacaoP1 + 1 - 0; // +1: UT é 0-indexado, "t" do PDF é o fim da UT
    int turnaroundP2 = finalizacaoP2 + 1 - 1;

    assert(finalizacaoP1 == 10); // t=11, bate com o PDF
    assert(turnaroundP1 == 11);

    assert(finalizacaoP2 == 25); // t=26 -- diverge do t=18 do PDF (ver comentário acima)
    assert(turnaroundP2 == 25);

    cout << "testarCenarioP1P2DoEnunciado: ok (turnaround P1=11 bate com o PDF; "
         << "P2=25 diverge do 17 do PDF por causa do BRPOS -- ver NOTES/perguntas.md)\n";
}

int main()
{
    testarEstourouQuantumFila1();
    testarPreempcaoFila1();
    testarCenarioP1P2DoEnunciado();
    cout << "todos os testes do scheduler passaram\n";
    return 0;
}
