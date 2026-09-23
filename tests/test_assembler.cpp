#include "assembler.h"
#include <cassert>
#include <iostream>

using namespace std;

// Confere o parse de tests/p1.asm contra o que o próprio arquivo descreve:
// LOAD valor / ADD #5 / STORE valor / SYSCALL 1 / SYSCALL 0, variavel valor=10
void testarP1()
{
    Programa p = parse("tests/p1.asm");

    assert(p.instrucoes.size() == 5);
    assert(p.variaveis.at("valor") == 10);

    assert(p.instrucoes[0].opcode == Opcode::LOAD);
    assert(p.instrucoes[0].tipoOperando == TipoOperando::DIRETO);
    assert(p.instrucoes[0].nomeVariavel == "valor");

    assert(p.instrucoes[1].opcode == Opcode::ADD);
    assert(p.instrucoes[1].tipoOperando == TipoOperando::IMEDIATO);
    assert(p.instrucoes[1].valorImediato == 5);

    assert(p.instrucoes[2].opcode == Opcode::STORE);
    assert(p.instrucoes[2].tipoOperando == TipoOperando::DIRETO);
    assert(p.instrucoes[2].nomeVariavel == "valor");

    assert(p.instrucoes[3].opcode == Opcode::SYSCALL);
    assert(p.instrucoes[3].indiceSyscall == 1);

    assert(p.instrucoes[4].opcode == Opcode::SYSCALL);
    assert(p.instrucoes[4].indiceSyscall == 0);

    cout << "p1: ok\n";
}

// Confere o parse de tests/p2.asm: 7 instrucoes (o label "loop:" nao conta
// como instrucao) e o alvoSalto do BRPOS resolvido pro indice certo
void testarP2()
{
    Programa p = parse("tests/p2.asm");

    assert(p.instrucoes.size() == 7);
    assert(p.variaveis.at("limite") == 3);
    assert(p.variaveis.at("temp") == 0);

    // indice 5 = "BRPOS loop"; deve apontar pro indice 1 ("SUB #1", logo apos "loop:")
    assert(p.instrucoes[5].opcode == Opcode::BRPOS);
    assert(p.instrucoes[5].alvoSalto == 1);

    cout << "p2: ok\n";
}

int main()
{
    testarP1();
    testarP2();
    cout << "todos os testes do assembler passaram\n";
    return 0;
}
