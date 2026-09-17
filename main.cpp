#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <sstream>

using namespace std;

// Representa os estados possíveis de um processo no ciclo de vida
enum State { READY, RUNNING, BLOCKED, FINISHED };

// Estrutura para armazenar as instruções da linguagem hipotética
struct Instruction {
    string opcode; // Ex: ADD, SUB, LOAD, SYSCALL
    string operand; // Ex: #1, variable, pontol
};

// Classe que representa um Processo (Bloco de Controle de Processo - PCB)
class Process {
public:
    string name;
    int arrival_time;
    int static_priority; // Prioridade de 1 a 5 para a Fila 1
    
    // Contexto de Execução
    int acc; // Acumulador onde as operações são realizadas
    int pc;  // Contador de programa (Program Counter)
    State current_state;
    
    // Métricas
    int time_in_cpu;
    int io_blocked_until; // Controla em qual UT o processo sai do I/O
    int turnaround_time;
    int wait_time;
    
    // Memória do Processo
    vector<Instruction> code;
    map<string, int> data; // Mapeia variáveis para seus valores inteiros

    map<string, int>labels; // Mapeia rótulos para endereços de instrução
    int current_quantum; // Controla o tempo gasto no quantum atual

    Process(string n, int arrival, int priority) {
        name = n;
        arrival_time = arrival;
        static_priority = priority;
        acc = 0;
        pc = 0;
        current_state = READY;
        time_in_cpu = 0;
        current_quantum = 0;
        io_blocked_until = -1;
    }
};

// Classe principal que gerencia o MLFQ e a CPU
class Scheduler {
private:
    int current_time; // Relógio global (UTs)
    
    // Fila 0: Round Robin Puro (Quantum = 2)
    queue<Process*> queue_0; 
    
    // Fila 1: Round Robin com Prioridade Estática (Quantum = 4)
    // Dica: Você precisará de uma estrutura que permita ordenação por prioridade e desempate FIFO
    vector<Process*> queue_1; 
    
    vector<Process*> blocked_list;
    Process* running_process;
    int current_quantum; // Controla o tempo gasto no quantum atual

public:
    Scheduler() {
        current_time = 0;
        running_process = nullptr;
        current_quantum = 0;
    }

    void insertFila1(Process* p, bool to_top) {
        auto it = queue_1.begin();

        while(it != queue_1.end()){
            if(p->static_priority < (*it)->static_priority){
                break;
            }
            else if(p->static_priority == (*it)->static_priority){
                // Se a prioridade for igual, mantém a ordem FIFO
                if(to_top) break;
            }
            ++it;
        }
        queue_1.insert(it, p);
    }

    

    // Processa 1 instrução do processo atual
    void executeInstruction(Process* p) {
        if (p->pc >= p->code.size()) return; // Fim do código (segurança)

        Instruction inst = p->code[p->pc];
        
        // Exemplo de execução das instruções aritméticas e de sistema
        if (inst.opcode == "LOAD") {
            // Verifica se é imediato (começa com #) ou variável direto
            if (inst.operand[0] == '#') {
                p->acc = stoi(inst.operand.substr(1)); // Extrai o valor após o '#'
            } else {
                p->acc = p->data[inst.operand]; // Carrega da memória de dados
            }
            p->pc++;
        } 
        else if (inst.opcode == "ADD") {
            // Lógica similar de soma imediata ou direta
            p->pc++;
        }
        else if (inst.opcode == "SYSCALL") {
            if (inst.operand == "0") {
                // Halt: Finaliza o processo
                p->current_state = FINISHED;
                // Calcular Turnaround aqui
            } 
            else if (inst.operand == "1" || inst.operand == "2") {
                // I/O: Bloqueia por 3 UTs exatas
                if (inst.operand == "1") cout << "[" << p->name << "] Impressão: " << p->acc << endl;
                
                p->current_state = BLOCKED;
                p->io_blocked_until = current_time + 3; // Retorna após 3 UTs
                blocked_list.push_back(p);
                running_process = nullptr; // Libera CPU
                p->pc++; // Prepara para a próxima instrução quando voltar
            }
        }
        // ... implementar BRPOS, BRZERO, BRNEG, etc ...
        
        p->time_in_cpu++;
        current_quantum++;
    }
};

int main() {
    // Aqui você fará o parser do arquivo .txt ou interativo para popular os processos
    // e criará o loop principal que chama scheduler.tick() até que todos os processos finalizem.
    cout << "EXECUTANDO SIMULAÇÃO" << endl;
    
    return 0;
}