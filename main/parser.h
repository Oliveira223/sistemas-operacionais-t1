#ifndef PARSER_H
#define PARSER_H

///////////////////////////////////////////

#include <string>
#include "programa.h"

using namespace std;

class Parser {
public:
    Programa carregar(const string& nomeArquivo);
};

///////////////////////////////////////////

#endif