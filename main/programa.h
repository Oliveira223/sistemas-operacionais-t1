#ifndef PROGRAMA_C
#define PROGRAMA_C

///////////////////////////////////////////
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Instrucoes{
    string opcode;
    string operando;
};

struct Programa{
    vector<Instrucoes> instrucoes;
    unordered_map<string, int> labels;
    unordered_map<string, int> primeiroDados;
};

///////////////////////////////////////////

#endif