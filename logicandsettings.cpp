#include "tictactoe.h"
// ==================== Game Settings and Start ====================
// (applyGameSettings(), startPvPWithNames())

void TicTacToe::applyGameSettings() {
    totalGames = totalGamesSpinBox->value();
    gamesToWin = gamesToWinSpinBox->value();

    if (gamesToWin > totalGames) {
        QMessageBox::warning(this, "Invalid Settings", "Games to win cannot be greater than total games!");
        return;
    }

    if (mode == 1) {
        // FIXED: Proper guest mode handling
        if (guestMode) {
            player1Name = "Player 1";
            player2Name = "Player 2";
            stackedWidget->setCurrentIndex(2);
            resetGame();
        } else {
            stackedWidget->setCurrentIndex(4);
        }
    } else {
        stackedWidget->setCurrentIndex(2);
        player1Name = "AI";
        player2Name = loggedInUser.isEmpty() ? "Player" : loggedInUser;
        resetGame();
    }
}

void TicTacToe::startPvPWithNames() {
    QString player2InputName = player2NameEdit->text();
    if (!player2InputName.isEmpty()) {
        player2Name = player2InputName;
    } else {
        player2Name = "Player 2";
    }
    player2NameEdit->clear();
    player1Name = loggedInUser.isEmpty() ? "Player 1" : loggedInUser;
    stackedWidget->setCurrentIndex(2); // Go to game screen
    resetGame();
}
// ==================== Core Game Logic ====================
// (resetGame(), makeMove(), makeAIMove(), easyMove(), mediumMove(), hardMove(), minimax())
void TicTacToe::resetGame() {
    // Check if the series is already over
    if (player1Wins >= gamesToWin || player2Wins >= gamesToWin ||
        (player1Wins + player2Wins + ties) >= totalGames) {
        currentSeriesId.clear(); // Clear series ID for new series
        backToModeSelection();
        return;
    }

    board = std::vector<char>(9, EMPTY);
    firstMoveMade = false;
    scoreboardVisible = false;
    scoreLabel->setVisible(scoreboardVisible);
    if (scoreboardToggleButton) {
        scoreboardToggleButton->setText("Show Scoreboard");
    }

    if (mode == 2) {
        player1Name = "AI";
        player2Name = loggedInUser.isEmpty() ? "Player" : loggedInUser;
    }

    updateScoreboard();

    // FIXED: Store the starting player when game begins
    std::random_device rd;
    std::mt19937 gen(rd());
    if (gen() % 2 == 0) {
        currentPlayer = PLAYER1;
        gameStartingPlayer = PLAYER1; // Store for later use
        if (mode == 2) {
            QTimer::singleShot(500, this, [this]() { makeAIMove(); });
        }
    } else {
        currentPlayer = PLAYER2;
        gameStartingPlayer = PLAYER2; // Store for later use
    }

    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton*>(buttonGroup->button(i));
        if (button) {
            button->setText("");
            button->setEnabled(true);
        }
    }
    updateStatus();
}

void TicTacToe::makeMove(int index) {
    if (board[index] != EMPTY) return;

    board[index] = currentPlayer;
    moveHistory.push_back(index);
    updateBoard();

    if (checkWin(currentPlayer)) {
        if (currentPlayer == PLAYER1) player1Wins++;
        else player2Wins++;

        // Save this individual game immediately
        QString winner = (currentPlayer == PLAYER1) ? player1Name : player2Name;
        saveIndividualGame(winner, "Win");

        updateScoreboard();

        // Check if series is complete
        if (player1Wins >= gamesToWin || player2Wins >= gamesToWin) {
            QString seriesWinner = player1Wins >= gamesToWin ? player1Name : player2Name;
            gameOver(QString("%1 wins the series %2-%3!")
                         .arg(seriesWinner)
                         .arg(qMax(player1Wins, player2Wins))
                         .arg(qMin(player1Wins, player2Wins)), true);
            return;
        }

        // Check if maximum games reached
        if ((player1Wins + player2Wins + ties) >= totalGames) {
            handleSeriesEnd();
            return;
        }

        gameOver(QString("%1 (%2) wins!").arg(winner).arg(currentPlayer));
        return;
    }

    if (checkTie()) {
        ties++;

        // Save this individual game as a tie
        saveIndividualGame("-", "Tie");

        updateScoreboard();

        // Check if maximum games reached
        if ((player1Wins + player2Wins + ties) >= totalGames) {
            handleSeriesEnd();
            return;
        }

        gameOver("It's a tie!");
        return;
    }

    currentPlayer = (currentPlayer == PLAYER1) ? PLAYER2 : PLAYER1;
    updateStatus();

    if (mode == 2 && currentPlayer == PLAYER1) {
        QTimer::singleShot(500, this, [this]() { makeAIMove(); });
    }
}

void TicTacToe::makeAIMove() {
    if (difficulty == 1) {
        easyMove();
    } else if (difficulty == 2) {
        mediumMove();
    } else {
        hardMove();
    }
}

void TicTacToe::easyMove() {
    std::vector<int> availableMoves;
    std::vector<int> moveScores;

    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            availableMoves.push_back(i);
            board[i] = PLAYER1;
            int score = minimax(0, false);
            board[i] = EMPTY;
            moveScores.push_back(score);
        }
    }

    std::vector<int> safeMoves;
    for (size_t i = 0; i < moveScores.size(); i++) {
        if (moveScores[i] != 1) {
            safeMoves.push_back(availableMoves[i]);
        }
    }

    std::vector<int> preferredMoves;
    for (size_t i = 0; i < safeMoves.size(); i++) {
        int idx = std::find(availableMoves.begin(), availableMoves.end(), safeMoves[i]) - availableMoves.begin();
        if (moveScores[idx] == -1) {
            preferredMoves.push_back(safeMoves[i]);
        }
    }

    if (!preferredMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, preferredMoves.size() - 1);
        makeMove(preferredMoves[dis(gen)]);
    } else if (!safeMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, safeMoves.size() - 1);
        makeMove(safeMoves[dis(gen)]);
    } else if (!availableMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, availableMoves.size() - 1);
        makeMove(availableMoves[dis(gen)]);
    }
}

void TicTacToe::mediumMove() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99);

    if (dis(gen) < 50) {
        hardMove();
    } else {
        easyMove();
    }
}


void TicTacToe::hardMove() {
    int bestScore = std::numeric_limits<int>::min();
    std::vector<int> bestMoves;

    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            board[i] = PLAYER1;
            int score = minimax(0, false);
            board[i] = EMPTY;

            if (score > bestScore) {
                bestScore = score;
                bestMoves.clear();
                bestMoves.push_back(i);
            } else if (score == bestScore) {
                bestMoves.push_back(i);
            }
        }
    }

    if (!bestMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, bestMoves.size() - 1);
        makeMove(bestMoves[dis(gen)]);
    }
}

int TicTacToe::minimax(int depth, bool isMaximizing) {
    if (checkWin(PLAYER1)) return 10 - depth;
    if (checkWin(PLAYER2)) return depth - 10;
    if (checkTie()) return 0;
    if (isMaximizing) {
        int bestScore = std::numeric_limits<int>::min();
        for (int i = 0; i < 9; i++) {
            if (board[i] == EMPTY) {
                board[i] = PLAYER1;
                int score = minimax(depth + 1, false);
                board[i] = EMPTY;
                bestScore = std::max(score, bestScore);
            }
        }
        return bestScore;
    } else {
        int bestScore = std::numeric_limits<int>::max();
        for (int i = 0; i < 9; i++) {
            if (board[i] == EMPTY) {
                board[i] = PLAYER2;
                int score = minimax(depth + 1, true);
                board[i] = EMPTY;
                bestScore = std::min(score, bestScore);
            }
        }
        return bestScore;
    }
}
// ==================== Mode Selection ====================
// (setPlayerVsPlayer(), setPlayerVsAI(), setDifficultyEasy(), setDifficultyMedium(), setDifficultyHard())
void TicTacToe::setPlayerVsPlayer() {
    mode = 1;
    stackedWidget->setCurrentIndex(5); // Go to game settings screen
}

void TicTacToe::setPlayerVsAI() {
    mode = 2;
    stackedWidget->setCurrentIndex(3); // Go to game settings screen
}

void TicTacToe::setDifficultyEasy() {
    difficulty = 1;
    stackedWidget->setCurrentIndex(5); // Go to game settings screen
}

void TicTacToe::setDifficultyMedium() {
    difficulty = 2;
    stackedWidget->setCurrentIndex(5); // Go to game settings screen
}

void TicTacToe::setDifficultyHard() {
    difficulty = 3;
    stackedWidget->setCurrentIndex(5); // Go to game settings screen
}
// ==================== Scoreboard ====================
// (updateScoreboard())
void TicTacToe::updateScoreboard() {
    QString scoreText = QString("<table width='100%'>"
                                "<tr><th align='left'>%1 (X)</th><th align='center'>Ties</th><th align='right'>%2 (O)</th></tr>"
                                "<tr><td align='left'><b>%3</b></td><td align='center'><b>%4</b></td><td align='right'><b>%5</b></td></tr>"
                                "<tr><td colspan='3' align='center'>Best of %6 (First to %7)</td></tr>"
                                "</table>")
                            .arg(player1Name)
                            .arg(player2Name)
                            .arg(player1Wins)
                            .arg(ties)
                            .arg(player2Wins)
                            .arg(totalGames)
                            .arg(gamesToWin);
    scoreLabel->setText(scoreText);
}

