#ifndef PROCESS_H
#define PROCESS_H

#include "assembler.h"
#include <vector>
#include <map>
#include <string>

// Em qual momento do ciclo de vida o processo está.
// Quem decide pra qual fila colocar o processo é o escalonador (Fase 3);
// quem muda esse valor pra BLOCKED/FINISHED é o próprio process, ao rodar SYSCALL.
enum class Estado
{
    PRONTO,     // esperando na fila pra rodar
    BLOQUEADO,  // fez SYSCALL 1/2, esperando 3 UTs de I/O
    FINALIZADO  // fez SYSCALL 0, não roda mais
};

// Uma instância em execução de um Programa. Diferente do Programa (fixo,
// resultado do parse), tudo aqui muda a cada instrução rodada — cada
// processo tem seu próprio acc/pc/memória, mesmo que dois processos
// rodem o mesmo .asm
struct Process
{
    // Identificador do processo (vem do arquivo de config). Existe porque o Process é copiado por valor entre as filas do scheduler o tempo todo. Vm ponteiro/índice pra "qual processo é esse" não sobrevive a essascópias, mas um campo dentro do próprio struct viaja junto.
    std::string nome;

    // Instruções do programa que esse processo executa (cópia de programa.instrucoes). Não muda durante a execução, só é consultada.
    std::vector<Instrucao> instrucoes;

    // Acumulador: único registrador dessa máquina hipotética, usado pelas instruções aritméticas e por LOAD/STORE.
    int acc;

    // Program counter: índice, em instrucoes, da próxima instrução a executar. Normalmente step() incrementa; saltos escrevem direto aqui.
    int pc;

    // Memória própria do processo. Começa como uma cópia de programa.variaveis (na criação), mas depois é independente: 
    //um STORE só altera a memória desse processo, nunca o Programa original.
    std::map<std::string, int> memoria;

    // Estado atual (ver enum Estado acima).
    Estado estado;

    // Prioridade estática (1-5), usada só pelas regras da Fila 1.
    int prioridade;
};

// Cria um Process a partir de um Programa já processado pelo assembler: copia as instruções, inicializa a memória a partir de programa.variaveis e zera acc/pc, deixando o processo pronto pra rodar (estado PRONTO).
Process criarProcesso(Programa programa, std::string nome, int prioridade);

// Executa a instrução em instrucoes[pc] sobre o processo e avança pc.
// Cada opcode altera acc/memoria/pc/estado de um jeito diferente ; quem decide QUANDO chamar step() é o scheduler.
void step(Process &processo);

#endif
