# Compilador usado e Flags de compilação
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Arquivos .cpp que entram na compilação
SRC = src/main.cpp src/assembler.cpp

# Headers dos quais o binário depende (se um mudar, recompila tudo)
HEADERS = src/assembler.h

# Nome do executável gerado
BIN = simulador


.PHONY: all test clean

# Alvo padrão ao rodar "make" sem argumento
all: $(BIN)

# Gera o executável a partir dos fontes; refaz se algum header mudar
$(BIN): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

# Compila e roda os testes do assembler (usa assembler.cpp, não main.cpp,
# porque test_assembler.cpp já tem seu próprio main)
test: tests/test_assembler.cpp src/assembler.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -Isrc tests/test_assembler.cpp src/assembler.cpp -o tests/test_assembler
	./tests/test_assembler

# Remove os executáveis gerados
clean:
	rm -f $(BIN) tests/test_assembler
