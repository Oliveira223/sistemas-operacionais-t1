CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -pedantic

SRC := src/assembler.cpp src/process.cpp src/scheduler.cpp
TARGET := simulador

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC) src/main.cpp
	$(CXX) $(CXXFLAGS) -Isrc $^ -o $@

test: test_assembler test_process test_scheduler
	./test_assembler
	./test_process
	./test_scheduler

test_assembler: $(SRC) tests/test_assembler.cpp
	$(CXX) $(CXXFLAGS) -Isrc $^ -o $@

test_process: $(SRC) tests/test_process.cpp
	$(CXX) $(CXXFLAGS) -Isrc $^ -o $@

test_scheduler: $(SRC) tests/test_scheduler.cpp
	$(CXX) $(CXXFLAGS) -Isrc $^ -o $@

clean:
	rm -f $(TARGET) test_assembler test_process test_scheduler
