#include <chrono>
#include <ctime>
#include <iostream>
#include <array>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <semaphore.h>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board;
    bool game_over;
    char winner;

    std::mutex mtx;

    sem_t turnoX;
    sem_t turnoO;

public:
    TicTacToe() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = ' ';
            }
        }

        game_over = false;
        winner = '-';

        static std::mt19937 sorteiaJogador(static_cast<unsigned int>(time(0)));
        static std::uniform_int_distribution<int> distr(0, 1);
        char inicial = (distr(sorteiaJogador) == 0) ? 'X' : 'O';

        if (inicial == 'X') {
            sem_init(&turnoX, 0, 1);
            sem_init(&turnoO, 0, 0);
        } else {
            sem_init(&turnoX, 0, 0);
            sem_init(&turnoO, 0, 1);
        }
    }

    ~TicTacToe() {
        sem_destroy(&turnoX);
        sem_destroy(&turnoO);
    }

    void display_board() const {
        std::cout << "\n";
        for (int i = 0; i < 3; i++) {
            std::cout << board[i][0] << "|" << board[i][1] << "|" << board[i][2] << "\n";
            if (i != 2) {
                std::cout << "-----\n";
            }
        }
        std::cout << "\n";
    }

    bool check_win(char player) const {
        for (int i = 0; i < 3; i++) {
            if (player == board[i][0] && player == board[i][1] && player == board[i][2]) {
                return true;
            }
        }

        for (int i = 0; i < 3; i++) {
            if (player == board[0][i] && player == board[1][i] && player == board[2][i]) {
                return true;
            }
        }

        if (player == board[0][0] && player == board[1][1] && player == board[2][2]) {
            return true;
        }

        if (player == board[0][2] && player == board[1][1] && player == board[2][0]) {
            return true;
        }

        return false;
    }

    bool check_draw() const {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_game_over() {
        std::lock_guard<std::mutex> lock(mtx);
        return game_over;
    }

    char get_winner() {
        std::lock_guard<std::mutex> lock(mtx);
        return winner;
    }

    bool make_move(char player, int row, int col) {
        sem_t* meuTurno = (player == 'X') ? &turnoX : &turnoO;
        sem_t* turnoOutro = (player == 'X') ? &turnoO : &turnoX;

        sem_wait(meuTurno);

        {
            std::lock_guard<std::mutex> lock(mtx);

            if (game_over) {
                sem_post(turnoOutro);
                return true;
            }

            if (row < 0 || row > 2 || col < 0 || col > 2) {
                sem_post(meuTurno);
                return false;
            }

            if (board[row][col] != ' ') {
                sem_post(meuTurno);
                return false;
            }

            board[row][col] = player;
            std::cout << "Jogador " << player << " jogou em (" << row << ", " << col << ")\n";
            display_board();

            if (check_win(player)) {
                winner = player;
                game_over = true;
                sem_post(turnoOutro);
                return true;
            }

            if (check_draw()) {
                winner = 'D';
                game_over = true;
                sem_post(turnoOutro);
                return true;
            }
        }

        sem_post(turnoOutro);
        return true;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game;
    char symbol;
    std::string strategy;
    std::mt19937 gen;

public:
    Player(TicTacToe& g, char s, const std::string& strat)
        : game(g), symbol(s), strategy(strat),
          gen(static_cast<unsigned>(
              std::chrono::steady_clock::now().time_since_epoch().count()) + s) {}

    void play() {
        while (!game.is_game_over()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            if (strategy == "sequential") {
                play_sequential();
            } else {
                play_random();
            }
        }
    }

private:
    void play_sequential() {
        for (int i = 0; i < 3 && !game.is_game_over(); i++) {
            for (int j = 0; j < 3 && !game.is_game_over(); j++) {
                if (game.make_move(symbol, i, j)) {
                    return;
                }
            }
        }
    }

    void play_random() {
        std::uniform_int_distribution<int> distr(0, 2);

        while (!game.is_game_over()) {
            int l = distr(gen);
            int c = distr(gen);

            if (game.make_move(symbol, l, c)) {
                return;
            }
        }
    }
};

// Função principal
int main() {
    TicTacToe tabuleiro;
    tabuleiro.display_board();

    Player X(tabuleiro, 'X', "sequential");
    Player O(tabuleiro, 'O', "random");

    std::thread Jogador1(&Player::play, &X);
    std::thread Jogador2(&Player::play, &O);

    Jogador1.join();
    Jogador2.join();

    char vencedor = tabuleiro.get_winner();

    if (vencedor == 'D') {
        std::cout << "Empate!\n";
    } else {
        std::cout << "Vencedor: " << vencedor << "\n";
    }

    return 0;
}
