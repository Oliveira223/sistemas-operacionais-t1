CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

SRC = main/main.cpp main/parser.cpp main/executor.cpp main/escala.cpp main/simulador.cpp
HEADERS = main/programa.h main/processo.h main/parser.h main/executor.h main/escala.h main/simula.h
BIN = simulador.exe

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

clean:
	rm -f $(BIN)