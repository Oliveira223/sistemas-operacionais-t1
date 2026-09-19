#include "parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

enum class Secao {
    NENHUMA,
    CODIGO,
    DADOS
};

static string trim(const string& texto) {
    const size_t inicio = texto.find_first_not_of(" \t\r\n");
    if(inicio == string::npos){
        return "";
    }

    const size_t fim = texto.find_last_not_of(" \t\r\n");
    return texto.substr(inicio, fim - inicio + 1);
}

Programa Parser::carregar(const string& nomeArquivo){
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo assembly: " + nomeArquivo);
    }

    Programa programa;
    Secao secao = Secao::NENHUMA;
    string linha;
    int numeroLinha = 0;

    while (getline(arquivo, linha)) {
        ++numeroLinha;
        linha = trim(linha);

        if (linha.empty() || linha[0] == '#') {
            continue;
        }

        if (linha == ".code") {
            secao = Secao::CODIGO;
            continue;
        }
        if (linha == ".endcode") {
            secao = Secao::NENHUMA;
            continue;
        }
        if (linha == ".data") {
            secao = Secao::DADOS;
            continue;
        }
        if (linha == ".enddata") {
            secao = Secao::NENHUMA;
            continue;
        }

        if (secao == Secao::CODIGO) {
            // Labels apontam para o indice da proxima instrucao e nao ocupam UT.
            stringstream inicioLinha(linha);
            string primeiroToken;
            inicioLinha >> primeiroToken;

            if (!primeiroToken.empty() && primeiroToken.back() == ':') {
                const string label = primeiroToken.substr(0, primeiroToken.size() - 1);

                if (label.empty()) {
                    throw runtime_error(
                        "Label invalido em " + nomeArquivo + ", linha " + to_string(numeroLinha));
                }
                if (programa.labels.count(label) != 0U) {
                    throw runtime_error(
                        "Label duplicado '" + label + "' em " + nomeArquivo +
                        ", linha " + to_string(numeroLinha));
                }

                programa.labels[label] = static_cast<int>(programa.instrucoes.size());
                linha = trim(linha.substr(primeiroToken.size()));

                if (linha.empty() || linha[0] == '#') {
                    continue;
                }
            }

            // Apenas opcode e operando fazem parte da instrucao; o restante da linha
            // pode ser comentario, inclusive apos operandos imediatos como #5.
            stringstream ss(linha);
            Instrucoes instrucao;
            ss >> instrucao.opcode >> instrucao.operando;

            if (instrucao.opcode.empty() || instrucao.operando.empty()) {
                throw runtime_error(
                    "Instrucao invalida em " + nomeArquivo +
                    ", linha " + to_string(numeroLinha));
            }

            programa.instrucoes.push_back(instrucao);
        }
        else if (secao == Secao::DADOS) {
            stringstream ss(linha);
            string nome;
            int valor = 0;

            if (!(ss >> nome >> valor)) {
                throw runtime_error(
                    "Declaracao de dado invalida em " + nomeArquivo +
                    ", linha " + to_string(numeroLinha));
            }
            if (programa.primeiroDados.count(nome) != 0U) {
                throw runtime_error(
                    "Variavel duplicada '" + nome + "' em " + nomeArquivo +
                    ", linha " + to_string(numeroLinha));
            }

            programa.primeiroDados[nome] = valor;
        }
        else {
            throw runtime_error(
                "Conteudo fora de .code/.data em " + nomeArquivo +
                ", linha " + to_string(numeroLinha));
        }
    }

    if (programa.instrucoes.empty()) {
        throw runtime_error("Programa sem instrucoes: " + nomeArquivo);
    }

    return programa;
}