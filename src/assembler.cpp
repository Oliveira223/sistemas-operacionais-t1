#include "assembler.h"
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

// Em qual parte do arquivo o parser está agora, pra saber como interpretar cada linha (uma linha "valor 10" significa coisas diferentes em .data e .code)
enum class Secao { NENHUMA, CODE, DATA };

// Tradução do texto do mnemônico (como aparece no .asm) pro Opcode correspondente
const map<string, Opcode> opcodesPorNome = {
    {"ADD", Opcode::ADD},
    {"SUB", Opcode::SUB},
    {"MULT", Opcode::MULT},
    {"DIV", Opcode::DIV},

    {"LOAD", Opcode::LOAD},
    {"STORE", Opcode::STORE},

    {"BRANY", Opcode::BRANY},   // salta sempre, incondicional
    {"BRPOS", Opcode::BRPOS},   // salta se acc > 0
    {"BRZERO", Opcode::BRZERO}, // salta se acc == 0
    {"BRNEG", Opcode::BRNEG},   // salta se acc < 0

    {"SYSCALL", Opcode::SYSCALL}
};

// Função que vai ler o programa .asm, linha por linha, pegar as informações necessárias e devolver a struct Programa
Programa parse(string caminho)
{
    Programa programa;
    ifstream arquivo(caminho);

    Secao secaoAtual = Secao::NENHUMA;
    string linha;

    // Mapa label -> índice da instrução, preenchido na 1ª passada (dentro do loop abaixo) e consultado na 2ª passada (depois do loop) pra resolver os saltos, porque um label pode ser usado antes de ser definido no arquivo
    map<string, int> rotulos;

    // Enquanto tiver linhas para pegar
    while (getline(arquivo, linha))
    {
        if (linha.empty())
        {
            continue; // linha em branco, nada a fazer
        }

        // linha.find(".code") procura o texto ".code" dentro de linha. Se achar, devolve a posição (um número); se não achar, devolve a flag npos, nativa da biblioteca
        if (linha.find(".code") != string::npos)
        {
            secaoAtual = Secao::CODE; // entrando na seção de instruções
            continue; // essa linha era só o marcador, não processa como conteúdo
        }
        if (linha.find(".endcode") != string::npos)
        {
            secaoAtual = Secao::NENHUMA; // saindo da seção de instruções
            continue;
        }
        if (linha.find(".data") != string::npos)
        {
            secaoAtual = Secao::DATA; // entrando na seção de variáveis
            continue;
        }
        if (linha.find(".enddata") != string::npos)
        {
            secaoAtual = Secao::NENHUMA; // saindo da seção de variáveis
            continue;
        }

        // Preencher dados, dependendo da seção atual
        if (secaoAtual == Secao::CODE)
        {
            istringstream streamDaLinha(linha);
            string opcodeTexto;
            streamDaLinha >> opcodeTexto;

            if (opcodeTexto.back() == ':')
            {
                // linha é só um label (ex: "loop:"), sem instrução própria. Guarda no mapa: a próxima instrução a entrar em programa.instrucoes vai ocupar o índice = tamanho atual do vetor (ainda não foi adicionada)
                string nomeLabel = opcodeTexto.substr(0, opcodeTexto.size() - 1); // tira o ":"
                rotulos[nomeLabel] = programa.instrucoes.size();
                continue;
            }

            string operandoTexto;
            streamDaLinha >> operandoTexto;

            Instrucao instrucao;
            instrucao.opcode = opcodesPorNome.at(opcodeTexto);

            // O operandoTexto pode significar coisas diferentes dependendo da instrução: SYSCALL já é um número (índice da chamada), vai direto pro indiceSyscall; imediato (ex: "#5") é número literal, tira o "#" e converte pro valorImediato; direto (ex: "valor") é nome de variável, guarda o texto puro em nomeVariavel. Saltos (BRANY/BRPOS/BRZERO/BRNEG) também caem no caso "direto" aqui, porque o nome do label não começa com "#". Isso é temporário e inofensivo: o próximo passo (resolução de labels) preenche o alvoSalto de verdade, e ninguém nunca lê nomeVariavel de uma instrução de salto.
            if (instrucao.opcode == Opcode::SYSCALL)
            {
                //stoi: string de numero para int
                instrucao.indiceSyscall = stoi(operandoTexto);
            }
            else if (operandoTexto.front() == '#') // front() = primeiro caractere da string
            {
                instrucao.tipoOperando = TipoOperando::IMEDIATO;
                instrucao.valorImediato = stoi(operandoTexto.substr(1)); // substr(1) = tudo menos o "#"
            }
            else
            {
                instrucao.tipoOperando = TipoOperando::DIRETO;
                instrucao.nomeVariavel = operandoTexto;
            }

            programa.instrucoes.push_back(instrucao);
        }
        else if (secaoAtual == Secao::DATA)
        {
            // "nome valor" -> programa.variaveis[nome] = valor. streamDaLinha >> valor (int) já converte o texto lido pra número sozinho
            istringstream streamDaLinha(linha);
            string nome;
            int valor;
            streamDaLinha >> nome >> valor;

            programa.variaveis[nome] = valor;
        }
    }

    // 2ª passada: o arquivo inteiro já foi lido, então "rotulos" está completo. Agora resolve o alvoSalto de cada instrução de salto, usando o nome do label que a 1ª parte deste laço guardou (temporariamente) em nomeVariavel.
    for (Instrucao& instrucao : programa.instrucoes)
    {
        bool ehSalto = instrucao.opcode == Opcode::BRANY || instrucao.opcode == Opcode::BRPOS ||
                       instrucao.opcode == Opcode::BRZERO || instrucao.opcode == Opcode::BRNEG;

        if (ehSalto)
        {
            instrucao.alvoSalto = rotulos.at(instrucao.nomeVariavel);
        }
    }

    return programa;
}
