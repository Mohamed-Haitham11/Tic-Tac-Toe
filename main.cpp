#include <QApplication>
#include "tictactoe.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TicTacToe window;
    window.show();

    return app.exec();
}
