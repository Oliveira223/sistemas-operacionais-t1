#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "parser.h"
#include "simula.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <arquivo_configuracao>\n";
        return 1;
    }

    try {
        ifstream config(argv[1]);
        if (!config.is_open()) {
            throw runtime_error("Nao foi possivel abrir o arquivo de configuracao.");
        }

        Parser parser;
        Simula simulador;
        unordered_set<string> nomes;

        string linha;
        int numeroLinha = 0;
        int quantidade = 0;

        // Formato de cada linha: nome arrival_time prioridade arquivo_asm
        while (getline(config, linha)) {
            ++numeroLinha;

            stringstream ss(linha);
            string nome;
            int arrivalTime = 0;
            int prioridade = 0;
            string arquivoAsm;

            if (!(ss >> nome)) {
                continue;
            }
            if (!nome.empty() && nome[0] == '#') {
                continue;
            }
            if (!(ss >> arrivalTime >> prioridade >> arquivoAsm)) {
                throw runtime_error(
                    "Linha invalida no arquivo de configuracao: " + to_string(numeroLinha));
            }

            if (arrivalTime < 0) {
                throw runtime_error("Arrival time invalido para " + nome);
            }
            if (prioridade < 1 || prioridade > 5) {
                throw runtime_error("Prioridade invalida para " + nome + " (use 1..5)");
            }
            if (nomes.count(nome) != 0U) {
                throw runtime_error("Nome de processo duplicado: " + nome);
            }
            nomes.insert(nome);

            Processo p;
            p.nome = nome;
            p.arrivalTime = arrivalTime;
            p.prioridade = prioridade;
            p.program = parser.carregar(arquivoAsm);

            // Cada processo recebe sua propria copia da area de dados.
            p.dados = p.program.primeiroDados;

            simulador.adicionarProcesso(p);
            ++quantidade;
        }

        if (quantidade == 0) {
            throw runtime_error("Nenhum processo foi carregado.");
        }

        simulador.executar();
    }
    catch (const exception& erro) {
        cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }

    return 0;
}
