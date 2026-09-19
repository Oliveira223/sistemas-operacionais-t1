#ifndef EXECUTOR_H
#define EXECUTOR_H

///////////////////////////////////////////

#include <string>
#include "processo.h"

enum class ResultExec{
    NORMAL,
    BLOQUEADO,
    FINALIZADO
};

class Executor{
public:
    ResultExec executar(Processo& processo);

private:
    int obterValorOp(const Processo& processo, const string& operando) const;
    int obterEnderecoLabel(const Processo& processo, const string& label) const;

};

///////////////////////////////////////////

#endif