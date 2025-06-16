#include "tictactoe.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QSqlError>

// ==================== Constructor ====================
TicTacToe::TicTacToe(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    connectToDatabase();
    createTablesIfNeeded();
    applyStyleSheet();
    resetGame();
}

QString currentUsername;

// ==================== Login / Logout ====================
void TicTacToe::handleLogin() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();
    currentUsername = username;

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Username and password cannot be empty!");
        return;
    }

    QSqlQuery query;
    // Fixed: Use 'password_hash' instead of 'password'
    query.prepare("SELECT password_hash, salt FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }

    if (query.next()) {
        QString storedHash = query.value(0).toString();  // This now gets password_hash
        QByteArray salt = QByteArray::fromHex(query.value(1).toString().toUtf8());

        QString computedHash = pbkdf2Hash(password, salt, 10000, 32);

        if (storedHash == computedHash) {
            loggedInUser = username;
            guestMode = false;
            stackedWidget->setCurrentIndex(1);

            QPushButton *historyButton = findChild<QPushButton*>("View Match History");
            if (historyButton) historyButton->show();

            QPushButton *deleteButton = findChild<QPushButton*>("DeleteAccountButton");
            if (deleteButton) deleteButton->show();

            usernameEdit->clear();
            passwordEdit->clear();
        } else {
            QMessageBox::warning(this, "Login Failed", "Invalid username or password!");
        }
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password!");
    }
}


void TicTacToe::logout() {
    if (mode != 1 && isMatchUnfinished() && !guestMode) {
        QString winner = (currentPlayer == PLAYER1) ? player2Name : "AI";
        saveMatchResult(winner, "Surrender");
    }

    QPushButton *deleteButton = findChild<QPushButton *>("DeleteAccountButton");
    if (deleteButton) deleteButton->hide();

    loggedInUser.clear();
    guestMode = false;
    usernameEdit->clear();
    passwordEdit->clear();
    stackedWidget->setCurrentIndex(0);
}
void TicTacToe::deleteAccount() {
    if (loggedInUser.isEmpty()) {
        QMessageBox::warning(this, "Delete Account", "You must be logged in to delete your account.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Account",
                                                              "Are you sure you want to permanently delete your account and all associated match history?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;

        // Delete match history
        query.prepare("DELETE FROM matches WHERE player1 = :username OR player2 = :username");
        query.bindValue(":username", loggedInUser);
        query.exec();

        // Delete user account
        query.prepare("DELETE FROM users WHERE username = :username");
        query.bindValue(":username", loggedInUser);
        if (query.exec()) {
            QMessageBox::information(this, "Account Deleted", "Your account and match history have been deleted.");
            logout();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete account: " + query.lastError().text());
        }
    }
}

// FIXED: Enhanced backToModeSelection to properly restore button connections
void TicTacToe::backToModeSelection() {
    if (inReplayMode) {
        // Restore original game state
        loggedInUser = originalGameState.loggedInUser;
        player1Wins = originalGameState.player1Wins;
        player2Wins = originalGameState.player2Wins;
        ties = originalGameState.ties;
        currentPlayer = originalGameState.currentPlayer;
        inReplayMode = false;

        // FIXED: Re-enable all game buttons and restore their connections
        for (int i = 0; i < 9; i++) {
            QPushButton *button = qobject_cast<QPushButton*>(buttonGroup->button(i));
            if (button) {
                button->setEnabled(true);
                // FIXED: Reconnect the button to the button group signal
                button->disconnect();
            }
        }

        // FIXED: Reconnect the buttonGroup signal
        connect(buttonGroup, &QButtonGroup::buttonClicked,
                this, [this](QAbstractButton* button) {
                    int id = buttonGroup->id(button);
                    handleButtonClick(id);
                });

        // Show surrender button again
        QPushButton *surrenderButton = findChild<QPushButton*>("SurrenderButton");
        if (surrenderButton) surrenderButton->show();

        // Restore original back button text
        QPushButton *backButton = findChild<QPushButton*>();
        if (backButton && backButton->text() == "← History") {
            backButton->setText("Back");
        }

        updateScoreboard();
        loadMatchHistory(); // Return to match history instead of mode selection
        return;
    }

    // Normal game end - reset scores
    player1Wins = 0;
    player2Wins = 0;
    ties = 0;
    updateScoreboard();
    stackedWidget->setCurrentIndex(1);
}

// FIXED: Enhanced loadRecordedMatch with complete button blocking
void TicTacToe::loadRecordedMatch() {
    int row = recordedMatchesTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Match Selected", "Please select a match to replay.");
        return;
    }

    QString matchId = recordedMatchesTable->item(row, 0)->text();
    QSqlQuery query;
    query.prepare("SELECT moves, player1, player2, starting_player, game_mode FROM matches WHERE id = :id");
    query.bindValue(":id", matchId);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to load match from database.");
        return;
    }

    QString moveStr = query.value(0).toString();
    QString originalPlayer1 = query.value(1).toString();
    QString originalPlayer2 = query.value(2).toString();
    QString startingPlayerStr = query.value(3).toString();
    QString gameMode = query.value(4).toString();
    char startingPlayer = startingPlayerStr.isEmpty() ? PLAYER1 : startingPlayerStr.at(0).toLatin1();

    // Store original game state to restore later
    originalGameState = {
        loggedInUser,
        player1Wins,
        player2Wins,
        ties,
        currentPlayer
    };

    // Set up player names based on original game mode
    if (gameMode == "PvAI") {
        if (originalPlayer1 == "AI") {
            player1Name = "AI";
            player2Name = originalPlayer2;
        } else {
            player1Name = originalPlayer1;
            player2Name = "AI";
        }
    } else {
        player1Name = originalPlayer1;
        player2Name = originalPlayer2;
    }

    QStringList moveTokens = moveStr.split(',', Qt::SkipEmptyParts);
    std::vector<int> replayMoves;
    for (const QString &move : moveTokens) {
        replayMoves.push_back(move.toInt());
    }

    // Complete reset for replay
    board = std::vector<char>(9, EMPTY);
    moveHistory.clear();
    player1Wins = 0;
    player2Wins = 0;
    ties = 0;
    currentPlayer = startingPlayer;

    // Set replay state
    this->replayMoves = replayMoves;
    this->replayIndex = 0;
    this->replayStartingPlayer = startingPlayer;
    this->replayGameMode = gameMode;
    this->inReplayMode = true;

    updateBoard();
    updateScoreboard();
    updateStatus();

    // FIXED: Completely disable all game buttons during replay
    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton*>(buttonGroup->button(i));
        if (button) {
            button->setEnabled(false);
            // FIXED: Also disconnect any existing click handlers during replay
            button->disconnect();
        }
    }

    if (!replayMoves.empty()) {
        statusLabel->setText(QString("Replay: %1 (%2) starts the game")
                                 .arg(startingPlayer == PLAYER1 ? player1Name : player2Name)
                                 .arg(startingPlayer));
        QTimer::singleShot(1000, this, &TicTacToe::replayNextMove);
    }

    // Hide surrender button and change back button text during replay
    QPushButton *surrenderButton = findChild<QPushButton*>("SurrenderButton");
    if (surrenderButton) surrenderButton->hide();

    // Change the existing back button text to indicate it returns to history
    QPushButton *backButton = findChild<QPushButton*>();
    if (backButton && backButton->text() == "Back") {
        backButton->setText("← History");
    }

    stackedWidget->setCurrentIndex(2);
}

// FIXED: Enhanced replayNextMove with proper completion handling
void TicTacToe::replayNextMove() {
    if (replayIndex >= static_cast<int>(replayMoves.size())) {
        statusLabel->setText("Replay completed");

        // Add a timer to return to match history after showing completion message
        QTimer::singleShot(2000, this, [this]() {
            // Restore original game state
            if (inReplayMode) {
                loggedInUser = originalGameState.loggedInUser;
                player1Wins = originalGameState.player1Wins;
                player2Wins = originalGameState.player2Wins;
                ties = originalGameState.ties;
                currentPlayer = originalGameState.currentPlayer;
                inReplayMode = false;

                // Show surrender button again
                QPushButton *surrenderButton = findChild<QPushButton*>("SurrenderButton");
                if (surrenderButton) surrenderButton->show();

                // Restore original back button text
                QPushButton *backButton = findChild<QPushButton*>();
                if (backButton && backButton->text() == "← History") {
                    backButton->setText("Back");
                }
            }

            updateScoreboard();
            loadMatchHistory(); // Return to match history
        });
        return;
    }

    // Calculate which player should make this move based on starting player
    char currentReplayPlayer = (replayIndex % 2 == 0) ? replayStartingPlayer :
                                   (replayStartingPlayer == PLAYER1 ? PLAYER2 : PLAYER1);

    int moveIndex = replayMoves[replayIndex];
    board[moveIndex] = currentReplayPlayer;
    updateBoard();

    // Determine player name for display
    QString playerName;
    if (replayGameMode == "PvAI") {
        if (currentReplayPlayer == PLAYER1) {
            playerName = (player1Name == "AI") ? "AI" : player1Name;
        } else {
            playerName = (player2Name == "AI") ? "AI" : player2Name;
        }
    } else {
        playerName = (currentReplayPlayer == PLAYER1) ? player1Name : player2Name;
    }

    bool gameEnded = false;

    // FIXED: Check for win first, then tie - order matters!
    if (checkWin(currentReplayPlayer)) {
        if (currentReplayPlayer == PLAYER1) player1Wins++;
        else player2Wins++;
        gameEnded = true;
        statusLabel->setText(QString("Replay: %1 (%2) wins this game!")
                                 .arg(playerName).arg(currentReplayPlayer));
    } else if (checkTie()) {
        // FIXED: Only increment ties, don't increment win counters for ties
        ties++;
        gameEnded = true;
        statusLabel->setText("Replay: This game is a tie!");
    } else {
        statusLabel->setText(QString("Replay: %1 (%2) moves to position %3")
                                 .arg(playerName).arg(currentReplayPlayer).arg(moveIndex + 1));
    }

    updateScoreboard();
    replayIndex++;

    if (gameEnded) {
        QTimer::singleShot(1500, this, [this]() {
            // FIXED: Check series completion properly
            if (player1Wins >= gamesToWin || player2Wins >= gamesToWin ||
                (player1Wins + player2Wins + ties) >= totalGames) {

                // FIXED: Determine series winner correctly
                QString seriesResult;
                if (player1Wins > player2Wins) {
                    seriesResult = QString("Replay: %1 wins the series %2-%3!")
                    .arg(player1Name)
                        .arg(player1Wins)
                        .arg(player2Wins);
                } else if (player2Wins > player1Wins) {
                    seriesResult = QString("Replay: %1 wins the series %2-%3!")
                    .arg(player2Name)
                        .arg(player2Wins)
                        .arg(player1Wins);
                } else {
                    seriesResult = QString("Replay: Series ended in a tie %1-%2!")
                    .arg(player1Wins)
                        .arg(player2Wins);
                }
                statusLabel->setText(seriesResult);
            } else {
                board = std::vector<char>(9, EMPTY);
                updateBoard();
                statusLabel->setText("Replay: Starting next game in series...");
                QTimer::singleShot(800, this, &TicTacToe::replayNextMove);
            }
        });
    } else {
        QTimer::singleShot(800, this, &TicTacToe::replayNextMove);
    }
}
// ==================== Win / Tie Check ====================
void TicTacToe::gameOver(const QString &message, bool seriesOver) {
    statusLabel->setText(message);
    if (!scoreboardVisible)
        toggleScoreboard();
    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton*>(buttonGroup->button(i));
        if (button) button->setEnabled(false);
    }

    if (!guestMode && seriesOver) {
        QString winner;
        QString result;
        if (player1Wins > player2Wins) {
            winner = player1Name;
            result = (player1Name == "AI") ? "Defeat" : "Win";
        } else if (player2Wins > player1Wins) {
            winner = player2Name;
            result = (player2Name == "AI") ? "Defeat" : "Win";
        } else {
            winner = "-";
            result = "Tie";
        }
    }

    if (seriesOver)
        QTimer::singleShot(3000, this, &TicTacToe::backToModeSelection);
    else
        QTimer::singleShot(2000, this, &TicTacToe::resetGame);
}


void TicTacToe::handleSeriesEnd() {
    if (player1Wins > player2Wins) {
        gameOver(QString("%1 wins the series %2-%3!")
                     .arg(player1Name)
                     .arg(player1Wins)
                     .arg(player2Wins), true);
    } else if (player2Wins > player1Wins) {
        gameOver(QString("%1 wins the series %2-%3!")
                     .arg(player2Name)
                     .arg(player2Wins)
                     .arg(player1Wins), true);
    } else {
        gameOver("Series ended in a tie!", true);
    }
}


bool TicTacToe::checkWin(char player) {
    for (int i = 0; i < 3; i++) {
        if (board[i*3] == player && board[i*3+1] == player && board[i*3+2] == player) return true;
    }
    for (int i = 0; i < 3; i++) {
        if (board[i] == player && board[i+3] == player && board[i+6] == player) return true;
    }
    if (board[0] == player && board[4] == player && board[8] == player) return true;
    if (board[2] == player && board[4] == player && board[6] == player) return true;
    return false;
}

bool TicTacToe::checkTie() {
    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) return false;
    }
    return true;
}

bool TicTacToe::isMatchUnfinished() {
    return !checkWin(PLAYER1) && !checkWin(PLAYER2) && !checkTie();
}

// ==================== Board / UI Updates ====================
void TicTacToe::updateBoard() {
    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
        if (button) {
            QChar symbol = board[i];
            button->setText(symbol == EMPTY ? "" : QString(symbol));
            if (symbol == 'X') {
                button->setStyleSheet(button->styleSheet() + "color: red; font-family: 'Georgia'; font-size: 28px;");
            } else if (symbol == 'O') {
                button->setStyleSheet(button->styleSheet() + "color: blue; font-family: 'Comic Sans MS'; font-size: 28px;");
            }
        }
    }
}

void TicTacToe::updateStatus() {
    QString playerName = (currentPlayer == PLAYER1) ? player1Name : player2Name;
    if (!loggedInUser.isEmpty()) {
        statusLabel->setText(QString("%1's turn (%2)").arg(playerName).arg(currentPlayer));
    } else {
        statusLabel->setText(QString("Player %1's turn").arg(currentPlayer));
    }
}

void TicTacToe::toggleScoreboard() {
    scoreboardVisible = !scoreboardVisible;
    scoreLabel->setVisible(scoreboardVisible);
    if (scoreboardToggleButton) {
        scoreboardToggleButton->setText(scoreboardVisible ? "Hide Scoreboard" : "Show Scoreboard");
    }
}

// ==================== Button Handler ====================
// FIXED: Enhanced handleButtonClick to completely block input during replay
void TicTacToe::handleButtonClick(int index) {
    // FIXED: Completely block all input during replay mode
    if (inReplayMode) {
        return; // Do nothing if in replay mode
    }

    if (board[index] != EMPTY || (mode == 2 && currentPlayer == PLAYER1)) return;
    makeMove(index);
}

void TicTacToe::connectToDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tictactoe.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", "Failed to open database!");
    }
}

void TicTacToe::createTablesIfNeeded() {
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database is not open.");
        return;
    }

    QSqlQuery query(db);

    // Create users table if not exists
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password_hash TEXT NOT NULL,"
            "salt TEXT NOT NULL"
            ")")) {
        QMessageBox::critical(this, "Database Error", "Failed to create users table: " + query.lastError().text());
        return;
    }

    // Create matches table if not exists (without starting_player, in case it already exists)
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS matches ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "player1 TEXT NOT NULL,"
            "player2 TEXT NOT NULL,"
            "winner TEXT,"
            "result TEXT,"
            "moves TEXT,"
            "timestamp TEXT,"
            "starting_player TEXT,"
            "game_mode TEXT,"
            "series_id TEXT,"           // New: Unique identifier for the series
            "game_number INTEGER,"      // New: Game number within the series
            "series_total INTEGER,"     // New: Total games in the series
            "series_target INTEGER"     // New: Games needed to win series
            ")")) {
        QMessageBox::critical(this, "Database Error", "Failed to create matches table: " + query.lastError().text());
        return;
    }

    // Check if starting_player column exists
    QSqlQuery pragmaQuery(db);
    bool hasStartingPlayer = false;
    if (pragmaQuery.exec("PRAGMA table_info(matches)")) {
        while (pragmaQuery.next()) {
            if (pragmaQuery.value(1).toString() == "starting_player") {
                hasStartingPlayer = true;
                break;
            }
        }
    }

    // Add starting_player column if missing
    if (!hasStartingPlayer) {
        QSqlQuery alterQuery(db);
        if (!alterQuery.exec("ALTER TABLE matches ADD COLUMN starting_player TEXT")) {
            // It's okay if this fails due to a race, but log it
            qDebug() << "Could not add starting_player column (may already exist):" << alterQuery.lastError().text();
        }
    }
}

void TicTacToe::saveMatchResult(const QString &winner, const QString &result) {
    // Don't save if in guest mode
    if (guestMode) {
        return;
    }

    if (!db.isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database is not open.");
        return;
    }

    QString moveSequence;
    for (int i = 0; i < static_cast<int>(moveHistory.size()); ++i) {
        moveSequence += QString::number(moveHistory[i]);
        if (i != static_cast<int>(moveHistory.size()) - 1)
            moveSequence += ",";
    }

    // FIXED: Use the stored starting player, not calculated from move history
    char startingPlayer = gameStartingPlayer;

    // Store the current game mode for proper replay
    QString gameMode = (mode == 2) ? "PvAI" : "PvP";

    QSqlQuery query;
    query.prepare("INSERT INTO matches "
                  "(player1, player2, winner, result, moves, timestamp, starting_player, game_mode) "
                  "VALUES (:player1, :player2, :winner, :result, :moves, :timestamp, :starting_player, :game_mode)");
    query.bindValue(":player1", player1Name);
    query.bindValue(":player2", player2Name);
    query.bindValue(":winner", winner);
    query.bindValue(":result", result);
    query.bindValue(":moves", moveSequence);
    query.bindValue(":timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":starting_player", QString(startingPlayer));
    query.bindValue(":game_mode", gameMode);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to save match result: " + query.lastError().text());
        return;
    }
}

void TicTacToe::saveIndividualGame(const QString &winner, const QString &result) {
    // Don't save if in guest mode
    if (guestMode) {
        return;
    }

    if (!db.isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database is not open.");
        return;
    }

    // Generate series ID if this is the first game
    if (currentSeriesId.isEmpty()) {
        currentSeriesId = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_") +
                          QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    }

    QString moveSequence;
    for (int i = 0; i < static_cast<int>(moveHistory.size()); ++i) {
        moveSequence += QString::number(moveHistory[i]);
        if (i != static_cast<int>(moveHistory.size()) - 1)
            moveSequence += ",";
    }

    QString gameMode = (mode == 2) ? "PvAI" : "PvP";
    int currentGameNumber = player1Wins + player2Wins + ties;

    QSqlQuery query;
    query.prepare("INSERT INTO matches "
                  "(player1, player2, winner, result, moves, timestamp, starting_player, game_mode, "
                  "series_id, game_number, series_total, series_target) "
                  "VALUES (:player1, :player2, :winner, :result, :moves, :timestamp, :starting_player, "
                  ":game_mode, :series_id, :game_number, :series_total, :series_target)");

    query.bindValue(":player1", player1Name);
    query.bindValue(":player2", player2Name);
    query.bindValue(":winner", winner);
    query.bindValue(":result", result);
    query.bindValue(":moves", moveSequence);
    query.bindValue(":timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":starting_player", QString(gameStartingPlayer));
    query.bindValue(":game_mode", gameMode);
    query.bindValue(":series_id", currentSeriesId);
    query.bindValue(":game_number", currentGameNumber);
    query.bindValue(":series_total", totalGames);
    query.bindValue(":series_target", gamesToWin);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to save game result: " + query.lastError().text());
        return;
    }

    // Clear move history for next game
    moveHistory.clear();
}

void TicTacToe::loadMatchHistory() {
    matchHistoryTable->clearContents();
    matchHistoryTable->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT player1, player2, winner, result, timestamp, series_id, game_number, series_total "
                  "FROM matches WHERE player1 = :user OR player2 = :user ORDER BY timestamp DESC");
    query.bindValue(":user", loggedInUser);
    query.exec();

    int row = 0;
    while (query.next()) {
        matchHistoryTable->insertRow(row);

        // Format display to show series information
        QString seriesInfo = QString("Game %1/%2 (Series: %3)")
                                 .arg(query.value(6).toInt())  // game_number
                                 .arg(query.value(7).toInt())  // series_total
                                 .arg(query.value(5).toString().right(8)); // last 8 chars of series_id

        QStringList displayData = {
            query.value(4).toString(), // timestamp
            query.value(0).toString(), // player1
            query.value(1).toString(), // player2
            query.value(2).toString(), // winner
            query.value(3).toString() + " (" + seriesInfo + ")" // result with series info
        };

        for (int col = 0; col < displayData.size(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(displayData[col]);
            item->setTextAlignment(Qt::AlignCenter);
            matchHistoryTable->setItem(row, col, item);
        }

        ++row;
    }

    stackedWidget->setCurrentIndex(6);
}

void TicTacToe::registerAccount() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Registration Failed", "Username and password cannot be empty!");
        return;
    }

    QByteArray salt = generateSalt(16);
    QString hashed = pbkdf2Hash(password, salt, 10000, 32);

    QSqlQuery query;
    // Fixed: Use 'password_hash' instead of 'password'
    query.prepare("INSERT INTO users (username, password_hash, salt) VALUES (:username, :password_hash, :salt)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", hashed);  // Changed from :password to :password_hash
    query.bindValue(":salt", QString(salt.toHex()));

    if (!query.exec()) {
        QMessageBox::warning(this, "Registration Failed", "Error: " + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Registration Success", "Account created successfully!");
    usernameEdit->clear();
    passwordEdit->clear();
}


void TicTacToe::guestLogin() {
    loggedInUser = "Player 1";
    guestMode = true;
    stackedWidget->setCurrentIndex(1);
    usernameEdit->clear();
    passwordEdit->clear();
}

void TicTacToe::handleSurrender() {
    QString winner;
    QString result = "Surrender";
    if (mode == 2) {
        winner = (currentPlayer == PLAYER2) ? "AI" : player2Name;
    } else {
        winner = (currentPlayer == PLAYER1) ? player2Name : player1Name;
    }
    saveMatchResult(winner, result);
    QMessageBox::information(this, "Surrender", QString::fromUtf8(u8"🏳️ %1 wins by surrender!").arg(winner));
    backToModeSelection();
}

QByteArray TicTacToe::generateSalt(int length) {
    QByteArray salt;
    for (int i = 0; i < length; ++i) {
        salt.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    return salt;
}

QString TicTacToe::pbkdf2Hash(const QString &password, const QByteArray &salt, int iterations, int dkLen) {
    QByteArray derivedKey;
    QByteArray passwordBytes = password.toUtf8();
    int hashLen = QCryptographicHash::hashLength(QCryptographicHash::Sha256);
    int blocks = (dkLen + hashLen - 1) / hashLen;

    for (int i = 1; i <= blocks; ++i) {
        QByteArray blockIndex;
        blockIndex.append((i >> 24) & 0xff);
        blockIndex.append((i >> 16) & 0xff);
        blockIndex.append((i >> 8) & 0xff);
        blockIndex.append(i & 0xff);

        QByteArray u = QCryptographicHash::hash(salt + blockIndex, QCryptographicHash::Sha256);
        QByteArray t = u;

        for (int j = 1; j < iterations; ++j) {
            u = QCryptographicHash::hash(u, QCryptographicHash::Sha256);
            for (int k = 0; k < t.size(); ++k)
                t[k] ^= u[k];
        }

        derivedKey += t;
    }

    return derivedKey.left(dkLen).toHex();
}
void TicTacToe::loadRecordedMatchesScreen() {
    recordedMatchesTable->clearContents();
    recordedMatchesTable->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT id, player1, player2, winner, result, timestamp FROM matches WHERE player1 = :user OR player2 = :user ORDER BY timestamp DESC");
    query.bindValue(":user", loggedInUser);
    query.exec();

    int row = 0;
    while (query.next()) {
        recordedMatchesTable->insertRow(row);
        for (int col = 0; col < 6; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            item->setTextAlignment(Qt::AlignCenter);
            recordedMatchesTable->setItem(row, col, item);
        }
        ++row;
    }

    stackedWidget->setCurrentIndex(7); // Show recorded match screen
}
