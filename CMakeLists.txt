cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(TicTacToeConcurrent)

# Define o padrão C++ para C++11 ou superior
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adiciona o executável
add_executable(tic_tac_toe src/tictactoe.cpp)

# Adiciona as flags de compilação para threads
find_package(Threads REQUIRED)
target_link_libraries(tic_tac_toe PRIVATE Threads::Threads)
