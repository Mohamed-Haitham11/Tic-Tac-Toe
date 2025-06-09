#include <QApplication>
#include "tictactoe.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TicTacToe game;
    game.resize(600, 700);
    game.show();
    return app.exec();
}

