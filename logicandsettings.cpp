#include "tictactoe.h"
#include <windows.h>
#include <psapi.h>
#include <QDebug>

// ==================== Game Settings and Start ====================
// (applyGameSettings(), startPvPWithNames())
// ..............................................test
void printMemoryUsage() {
    PROCESS_MEMORY_COUNTERS memInfo;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo))) {
        SIZE_T physMemUsed = memInfo.WorkingSetSize;
        qDebug() << "Memory usage (MB):" << physMemUsed / (1024.0 * 1024.0);
    }
}
char TicTacToe::getBoardState(int row, int col) {
    // This converts the 2D row/col to the correct 1D index for your board vector
    int index = row * 3 + col;
    if (index >= 0 && index < board.size()) {
        return this->board[index];
    }
    return EMPTY; // EMPTY is defined as ' ' in your tictactoe.h
}
void TicTacToe::setTestBoardState(const std::vector<char>& testBoard, char nextPlayer) {
    this->board = testBoard;
    this->currentPlayer = nextPlayer;
}

char TicTacToe::getCurrentPlayer() {
    return this->currentPlayer;
}
//.................................test
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
        QPushButton button = qobject_cast<QPushButton>(buttonGroup->button(i));
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

    if (checkWin(currentPlayer, this->board)) {
        // Calculate game number BEFORE updating scores
        int actualGameNumber = player1Wins + player2Wins + ties + 1;

        QString winner;
        QString result;

        if (mode == 2) { // Player vs AI
            if (currentPlayer == PLAYER1) { // AI wins
                player1Wins++; // AI wins count goes to player1Wins (since AI is player1)
                winner = "AI";
                result = "🤖 AI Victory"; // AI wins with robot emoji
            } else { // Player wins
                player2Wins++; // Player wins count goes to player2Wins
                winner = player2Name;
                result = "🏆 Victory"; // Player wins with trophy
            }
        } else { // Player vs Player
            if (currentPlayer == PLAYER1) player1Wins++;
            else player2Wins++;
            winner = (currentPlayer == PLAYER1) ? player1Name : player2Name;
            result = "🥇 Win"; // Standard win with gold medal
        }

        // Save this individual game immediately with correct game number
        saveIndividualGameWithNumber(winner, result, actualGameNumber);

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

    if (checkTie(this->board)) {
        // Calculate game number BEFORE updating scores
        int actualGameNumber = player1Wins + player2Wins + ties + 1;

        ties++;

        // Save this individual game as a tie with expressive emoji
        saveIndividualGameWithNumber("-", "🤝 Tie", actualGameNumber);

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
    printMemoryUsage();
}

void TicTacToe::makeAIMove() {
    int move = -1;
    switch (difficulty) {
    case 1: // Easy
        move = easyMove();
        break;
    case 2: // Medium
        move = mediumMove();
        break;
    case 3: // Hard
        move = hardMove();
        break;
    default:
        move = hardMove(); // Default to hard
        break;
    }

    if (move != -1 && move >= 0 && move < 9 && board[move] == EMPTY) {
        makeMove(move);
    }
    printMemoryUsage();
}

int TicTacToe::easyMove() {
    std::vector<int> availableMoves;
    std::vector<bool> isImmediateWin;
    std::vector<bool> isBlockingMove;

    // Find all available moves and categorize them
    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            availableMoves.push_back(i);

            // Check if this move results in immediate win for AI
            std::vector<char> tempBoardWin = board;
            tempBoardWin[i] = PLAYER1; // AI makes the move
            bool immediateWin = checkWin(PLAYER1, tempBoardWin); // <-- FIXED: Pass the temp board
            isImmediateWin.push_back(immediateWin);

            // Check if placing a piece here would block an opponent's win
            std::vector<char> tempBoardBlock = board;
            tempBoardBlock[i] = PLAYER2; // What if the opponent played here?
            bool opponentWouldWin = checkWin(PLAYER2, tempBoardBlock); // <-- FIXED: Pass the temp board
            isBlockingMove.push_back(opponentWouldWin);
        }
    }

    // Count moves that are neither winning nor blocking
    std::vector<int> neutralMoves;
    for (size_t i = 0; i < availableMoves.size(); i++) {
        if (!isImmediateWin[i] && !isBlockingMove[i]) {
            neutralMoves.push_back(availableMoves[i]);
        }
    }

    // Priority 1: If there are neutral moves (neither winning nor blocking), choose one
    if (!neutralMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, neutralMoves.size() - 1);
        return neutralMoves[dis(gen)];
    }

    // Priority 2: If no neutral moves, prefer non-winning moves (even if they block)
    std::vector<int> nonWinningMoves;
    for (size_t i = 0; i < availableMoves.size(); i++) {
        if (!isImmediateWin[i]) {
            nonWinningMoves.push_back(availableMoves[i]);
        }
    }

    if (!nonWinningMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, nonWinningMoves.size() - 1);
        return nonWinningMoves[dis(gen)];
    }

    // Priority 3: If ALL moves result in immediate win, AI must win (no choice)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, availableMoves.size() - 1);
    return availableMoves[dis(gen)];
}
int TicTacToe::mediumMove() {
    std::vector<int> availableMoves;
    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            availableMoves.push_back(i);
        }
    }

    if (availableMoves.empty()) {
        return -1; // No move available
    }

    // 50% chance to play optimally, 50% chance to play the worst move
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> coinFlip(0, 1);

    if (coinFlip(gen) == 0) {
        // Play optimally using minimax
        // Count filled cells to determine if this is the first AI move
        int filledCount = 0;
        for (char c : board) {
            if (c != EMPTY) filledCount++;
        }

        // First move logic (only for AI's first move)
        if ((filledCount == 0) || (filledCount == 1 && !firstMoveMade)) {
            if (filledCount == 0) {
                // AI is starting - choose a random corner
                std::vector<int> corners = {0, 2, 6, 8};
                std::uniform_int_distribution<> dis(0, corners.size() - 1);
                return corners[dis(gen)];
            } else {
                // Player started - choose center if available, otherwise corner
                if (board[4] == EMPTY) {
                    return 4;
                } else {
                    std::vector<int> corners = {0, 2, 6, 8};
                    for (int corner : corners) {
                        if (board[corner] == EMPTY) {
                            return corner;
                        }
                    }
                }
            }
        }

        // Standard minimax algorithm for subsequent moves (optimal play)
        int bestScore = std::numeric_limits<int>::min();
        int bestMove = -1;
        for (int i = 0; i < 9; i++) {
            if (board[i] == EMPTY) {
                std::vector<char> tempBoard = board; // Create a copy
                tempBoard[i] = PLAYER1; // AI's move
                int score = minimax(tempBoard, false);
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = i;
                }
            }
        }
        return bestMove != -1 ? bestMove : availableMoves[0];
    } else {
        // Play the worst move using minimax (choose move with lowest score)
        int worstScore = std::numeric_limits<int>::max();
        int worstMove = -1;
        for (int i = 0; i < 9; i++) {
            if (board[i] == EMPTY) {
                std::vector<char> tempBoard = board; // Create a copy
                tempBoard[i] = PLAYER1; // AI's move
                int score = minimax(tempBoard, false);
                if (score < worstScore) {
                    worstScore = score;
                    worstMove = i;
                }
            }
        }
        return worstMove != -1 ? worstMove : availableMoves[0]; // Fallback to first available move
    }
}

int TicTacToe::hardMove() {
    int move = -1;

    if (!firstMoveMade) {
        // Count filled cells to check who started
        int filledCount = 0;
        for (char c : board) {
            if (c != EMPTY) filledCount++;
        }

        // If the board is empty, AI is starting (choose a random corner)
        if (filledCount == 0) {
            std::vector<int> corners = {0, 2, 6, 8};
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, corners.size() - 1);
            move = corners[dis(gen)];
        } else {
            // Otherwise, the first move is by the player:
            // Always choose the center if available, otherwise fallback (choose one of the corners)
            if (board[4] == EMPTY) {
                move = 4;
            } else {
                std::vector<int> corners = {0, 2, 6, 8};
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, corners.size() - 1);
                move = corners[dis(gen)];
            }
        }
        firstMoveMade = true;
    } else {
        // Standard minimax algorithm for subsequent moves
        int bestScore = std::numeric_limits<int>::min();
        for (int i = 0; i < 9; i++) {
            if (board[i] == EMPTY) {
                std::vector<char> tempBoard = board; // Create a copy
                tempBoard[i] = PLAYER1; // AI's move
                int score = minimax(tempBoard, false);
                if (score > bestScore) {
                    bestScore = score;
                    move = i;
                }
            }
        }
    }
    return move;
}

// Updated minimax function to handle dynamic AI player
int TicTacToe::minimax(std::vector<char> &tempBoard, bool isMaximizing) {
    // This now checks the hypothetical tempBoard directly.
    bool aiWins = checkWin(PLAYER1, tempBoard);
    bool playerWins = checkWin(PLAYER2, tempBoard);
    bool tie = checkTie(tempBoard);

    if (aiWins) return 1;
    // ... rest of function
    if (playerWins) return -1;  // Player wins
    if (tie) return 0;          // Tie

    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (int i = 0; i < 9; i++) {
        if (tempBoard[i] == EMPTY) {
            tempBoard[i] = isMaximizing ? PLAYER1 : PLAYER2;
            int score = minimax(tempBoard, !isMaximizing);
            tempBoard[i] = EMPTY;
            bestScore = isMaximizing ? std::max(score, bestScore) : std::min(score, bestScore);
        }
    }
    return bestScore;
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