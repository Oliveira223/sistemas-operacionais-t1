#include "process.h"
#include <cassert> // assert(condição): não faz nada se for true; se for false, aborta o programa na hora e imprime arquivo/linha/condição que falhou
#include <iostream>

using namespace std;

// Roda tests/p1.asm inteiro passo a passo e confere cada instrução contra o que o próprio arquivo descreve: LOAD/ADD/STORE/SYSCALL 1/SYSCALL 0
void testarP1()
{
    Programa programa = parse("tests/p1.asm");
    Process processo = criarProcesso(programa, "P1", 1);

    step(processo); // LOAD valor
    assert(processo.acc == 10);

    step(processo); // ADD #5
    assert(processo.acc == 15);

    step(processo); // STORE valor
    assert(processo.memoria.at("valor") == 15);

    step(processo); // SYSCALL 1
    assert(processo.estado == Estado::BLOQUEADO);
    assert(processo.pc == 4);

    step(processo); // SYSCALL 0
    assert(processo.estado == Estado::FINALIZADO);
    assert(processo.pc == 5);

    cout << "p1: ok\n";
}

// Roda tests/p2.asm: laço SUB/STORE/SYSCALL 1/LOAD/BRPOS repete 3x (acc vai de 3 até 0, decrementando 1 por iteração), depois sai do laço e finaliza
void testarP2()
{
    Programa programa = parse("tests/p2.asm");
    Process processo = criarProcesso(programa, "P2", 5);

    step(processo); // LOAD limite
    assert(processo.acc == 3);

    for (int iteracao = 0; iteracao < 3; iteracao++)
    {
        int accEsperado = 2 - iteracao; // 2, 1, 0

        step(processo); // SUB #1
        assert(processo.acc == accEsperado);

        step(processo); // STORE temp
        assert(processo.memoria.at("temp") == accEsperado);

        step(processo); // SYSCALL 1
        assert(processo.estado == Estado::BLOQUEADO);

        step(processo); // LOAD temp
        assert(processo.acc == accEsperado);

        step(processo); // BRPOS loop
        if (accEsperado > 0)
        {
            assert(processo.pc == 1); // saltou de volta pro início do laço
        }
        else
        {
            assert(processo.pc == 6); // acc chegou a 0: não salta, segue pro SYSCALL 0
        }
    }

    step(processo); // SYSCALL 0
    assert(processo.estado == Estado::FINALIZADO);

    cout << "p2: ok\n";
}

int main()
{
    testarP1();
    testarP2();
    cout << "todos os testes do process passaram\n";
    return 0;
}
