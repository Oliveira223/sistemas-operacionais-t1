#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <vector>
#include <map>

// Criar enum para representar mnemônicos
enum class Opcode
{
    ADD, SUB, MULT, DIV,
    LOAD, STORE,
    BRANY, BRPOS, BRZERO, BRNEG,
    SYSCALL
};

// Como o operando de uma instrução deve ser interpretado na execução: número literal (#5) ou nome de variável a buscar na memória do processo. Só um deve ser usado por vez
enum class TipoOperando
{
    IMEDIATO, // #N
    DIRETO    // endereço
};

// Juntar Opcode e TipoOperando em uma unica struct
struct Instrucao
{
    // Pega valores
    Opcode opcode;
    TipoOperando tipoOperando;

    //Se tipoOperando == IMEDIATO
    int valorImediato;

    //Se tipoOperando == DIRETO
    std::string nomeVariavel;

    // Campo especial para syscall, qual chamada de sistema fazer (0 = encerrar / 1 = imprimir / 2 = ler)
    int indiceSyscall;

    // Pra onde pular. Usado só por BRANY/BRPOS/BRZERO/BRNEG: índice, na lista de instruções,
    int alvoSalto;

};


// O que parse() vai devolver depois de ler um arquivo .asm por completo:
struct Programa
{
    // Instruções do .code, na ordem de execução, com labels já resolvidos em índices
    std::vector<Instrucao> instrucoes;

    // Valor inicial de cada variável do .data (nome -> valor), usado só pra montar a memória do processo no início. Exemplo: .data / valor 10 / Vira: variaveis["valor"] = 10

    std::map<std::string, int> variaveis;

};

// Por fim, a assinatura da função parse. Recebe o caminho onde está o arquivo e devolve um Programa pronto
Programa parse(std::string caminho);

#endif
