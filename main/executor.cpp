#include "executor.h"
#include <iostream>
#include <stdexcept>

int Executor::obterValorOp(const Processo& processo, const string& operando) const{
    if(!operando.empty() && operando[0] == '#'){
        try {
            return stoi(operando.substr(1));
        }
        catch (const exception&){
            throw runtime_error("Operando imediato invalido: " + operando);
        }
    }

    const auto it = processo.dados.find(operando);
    if(it == processo.dados.end()){
        throw runtime_error("Variavel '" + operando + "' nao encontrada no processo " + processo.nome);
    }
    return it->second;
}

int Executor::obterEnderecoLabel(const Processo& processo, const string& label) const{
    const auto it = processo.program.labels.find(label);
    if(it == processo.program.labels.end()){
        throw runtime_error("Label '" + label + "' nao encontrado no processo " + processo.nome);
    }
    return it->second;
}

ResultExec Executor::executar(Processo& processo){
    if (processo.programCounter < 0 ||
        processo.programCounter >= static_cast<int>(processo.program.instrucoes.size())) {
        throw runtime_error("PC fora da area de codigo no processo " + processo.nome);
    }

    const Instrucoes& instrucao = processo.program.instrucoes[processo.programCounter];
    const string& opcode = instrucao.opcode;
    const string& operando = instrucao.operando;

    if (opcode == "LOAD") {
        processo.acc = obterValorOp(processo, operando);
        ++processo.programCounter;
        return ResultExec::NORMAL;
    }

    if (opcode == "STORE") {
        if (!operando.empty() && operando[0] == '#') {
            throw runtime_error("STORE nao aceita enderecamento imediato no processo " + processo.nome);
        }

        auto it = processo.dados.find(operando);
        if (it == processo.dados.end()) {
            throw runtime_error(
                "Variavel '" + operando + "' nao encontrada no processo " + processo.nome);
        }

        it->second = processo.acc;
        ++processo.programCounter;
        return ResultExec::NORMAL;
    }

    if (opcode == "ADD") {
        processo.acc += obterValorOp(processo, operando);
        ++processo.programCounter;
        return ResultExec::NORMAL;
    }

    if (opcode == "SUB") {
        processo.acc -= obterValorOp(processo, operando);
        ++processo.programCounter;
        return ResultExec::NORMAL;
    }

    if (opcode == "MULT") {
        processo.acc *= obterValorOp(processo, operando);
        ++processo.programCounter;
        return ResultExec::NORMAL;
    }

    if (opcode == "DIV") {
        const int valor = obterValorOp(processo, operando);
        if (valor == 0) {
            throw runtime_error("Divisao por zero no processo " + processo.nome);
        }
        processo.acc /= valor;
        ++processo.programCounter;
        return ResultExec::NORMAL;
    }

    if (opcode == "BRANY") {
        processo.programCounter = obterEnderecoLabel(processo, operando);
        return ResultExec::NORMAL;
    }

    if (opcode == "BRPOS" || opcode == "BRZERO" || opcode == "BRNEG") {
        bool deveSaltar = false;
        if (opcode == "BRPOS") {
            deveSaltar = processo.acc > 0;
        }
        else if (opcode == "BRZERO") {
            deveSaltar = processo.acc == 0;
        }
        else {
            deveSaltar = processo.acc < 0;
        }

        if (deveSaltar) {
            processo.programCounter = obterEnderecoLabel(processo, operando);
        }
        else {
            ++processo.programCounter;
        }
        return ResultExec::NORMAL;
    }

    if (opcode == "SYSCALL") {
        int indice = -1;
        try {
            indice = stoi(operando);
        }
        catch (const exception&) {
            throw runtime_error("Indice de SYSCALL invalido no processo " + processo.nome);
        }

        if (indice == 0) {
            return ResultExec::FINALIZADO;
        }

        if (indice == 1) {
            cout << "[" << processo.nome << "] Impressao: " << processo.acc << '\n';
            // O PC avanca antes do bloqueio para que a SYSCALL nao seja repetida ao retornar.
            ++processo.programCounter;
            return ResultExec::BLOQUEADO;
        }

        if (indice == 2) {
            int valor = 0;
            cout << "[" << processo.nome << "] Digite um inteiro: ";
            if (!(cin >> valor)) {
                throw runtime_error("Falha na leitura de inteiro para o processo " + processo.nome);
            }

            // A leitura e armazenada no acumulador da maquina hipotetica.
            processo.acc = valor;
            ++processo.programCounter;
            return ResultExec::BLOQUEADO;
        }

        throw runtime_error("SYSCALL invalida no processo " + processo.nome + ": " + operando);
    }

    throw runtime_error("Instrucao desconhecida no processo " + processo.nome + ": " + opcode);
}