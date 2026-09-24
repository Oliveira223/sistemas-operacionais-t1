#include "process.h"

// Função auxiliar, static para existir apenas dentro desse codigo. Devolve o valor que a instrução referencia: valorImediato (#N) ou o que está guardado em memoria[nomeVariavel] (endereço direto). Usado por aritmética, LOAD e (depois) pelos saltos - todos leem um valor desse jeito.
static int obterValor(const Instrucao &instrucao, const Process &processo)
{
    if (instrucao.tipoOperando == TipoOperando::IMEDIATO)
    {
        return instrucao.valorImediato;
    }

    // .at() em vez de []: processo é const aqui (operator[] de map não compila em referência const), e além disso .at() lança exceção se a variável não existir, em vez de criar uma entrada nova com valor 0.
    return processo.memoria.at(instrucao.nomeVariavel);
}

Process criarProcesso(Programa programa, std::string nome, int prioridade)
{
    Process processo;

    processo.nome = nome;
    processo.instrucoes = programa.instrucoes;
    processo.memoria = programa.variaveis; // copia: mudanças aqui não afetam o Programa original

    processo.acc = 0;
    processo.pc = 0;
    processo.estado = Estado::PRONTO;
    processo.prioridade = prioridade;

    return processo;
}

void step(Process &processo)
{
    const Instrucao &instrucao = processo.instrucoes[processo.pc];

    switch (instrucao.opcode)
    {
        case Opcode::ADD:  processo.acc += obterValor(instrucao, processo); break;
        case Opcode::SUB:  processo.acc -= obterValor(instrucao, processo); break;
        case Opcode::MULT: processo.acc *= obterValor(instrucao, processo); break;
        case Opcode::DIV:  processo.acc /= obterValor(instrucao, processo); break;

        case Opcode::LOAD: processo.acc = obterValor(instrucao, processo); break;

        case Opcode::STORE: processo.memoria.at(instrucao.nomeVariavel) = processo.acc; break;

        // return em vez de break: quando o salto é tomado, pc já vira o alvo e a função sai aqui, sem passar pelo pc++ padrão do final (senão pularíamos pro alvo e ainda andaríamos mais uma casa).
        case Opcode::BRANY:
            processo.pc = instrucao.alvoSalto;
            return;

        case Opcode::BRPOS:
            if (processo.acc > 0)
            {
                processo.pc = instrucao.alvoSalto;
                return;
            }
            break;

        case Opcode::BRZERO:
            if (processo.acc == 0)
            {
                processo.pc = instrucao.alvoSalto;
                return;
            }
            break;

        case Opcode::BRNEG:
            if (processo.acc < 0)
            {
                processo.pc = instrucao.alvoSalto;
                return;
            }
            break;

        // indiceSyscall: 0=halt, 1=print, 2=read. O countdown de 3 UT do bloqueio é responsabilidade do scheduler
        case Opcode::SYSCALL:
            if (instrucao.indiceSyscall == 0)
            {
                processo.estado = Estado::FINALIZADO;
            }
            else
            {
                processo.estado = Estado::BLOQUEADO;
            }
            break;
    }

    processo.pc++;
}
