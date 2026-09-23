# Compilador usado e Flags de compilação
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Arquivos .cpp que entram na compilação
SRC = src/main.cpp
# Headers dos quais o binário depende (se um mudar, recompila tudo)
HEADERS =
# Nome do executável gerado
BIN = simulador


.PHONY: all clean

# Alvo padrão ao rodar "make" sem argumento
all: $(BIN)

# Gera o executável a partir dos fontes; refaz se algum header mudar
$(BIN): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

# Remove o executável gerado
clean:
	rm -f $(BIN)
