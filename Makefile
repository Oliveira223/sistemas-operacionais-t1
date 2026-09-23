# Compilador usado e Flags de compilação
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Arquivos .cpp que entram na compilação
SRC = src/main.cpp src/assembler.cpp

# Headers dos quais o binário depende (se um mudar, recompila tudo)
HEADERS = src/assembler.h src/process.h

# Nome do executável gerado
BIN = simulador


.PHONY: all test clean

# Alvo padrão ao rodar "make" sem argumento
all: $(BIN)

# Gera o executável a partir dos fontes; refaz se algum header mudar
$(BIN): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

# Compila e roda os testes do assembler e do process (cada test_*.cpp já
# tem seu próprio main, por isso não entram no $(SRC) do binário principal)
test: tests/test_assembler.cpp tests/test_process.cpp src/assembler.cpp src/process.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -Isrc tests/test_assembler.cpp src/assembler.cpp -o tests/test_assembler
	./tests/test_assembler
	$(CXX) $(CXXFLAGS) -Isrc tests/test_process.cpp src/assembler.cpp src/process.cpp -o tests/test_process
	./tests/test_process

# Remove os executáveis gerados
clean:
	rm -f $(BIN) tests/test_assembler tests/test_process
