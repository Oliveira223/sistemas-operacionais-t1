cmake_minimum_required(VERSION 3.10)
project(SimuladorSO CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_executable(simulador
    main/main.cpp
    main/parser.cpp
    main/executor.cpp
    main/escala.cpp
    main/simulador.cpp
)

# Adicionando o diretorio main aos includes
target_include_directories(simulador PRIVATE main)
