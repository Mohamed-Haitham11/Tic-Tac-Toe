#include "tictactoe.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QSqlError>

// ==================== Login / Logout ====================
void TicTacToe::handleLogin() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Username and password cannot be empty!");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT password_hash, salt FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }

    if (query.next()) {
        QString storedHash = query.value(0).toString();
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

    if (isRecording) {
        stopVideoRecording();
    }

    if (videoPlayer && videoPlayer->playbackState() == QMediaPlayer::PlayingState) {
        stopVideoPlayback();
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

        query.prepare("SELECT video_path FROM matches WHERE player1 = :username OR player2 = :username");
        query.bindValue(":username", loggedInUser);
        if (query.exec()) {
            while (query.next()) {
                QString videoPath = query.value(0).toString();
                if (!videoPath.isEmpty()) {
                    QFile::remove(videoPath);
                }
            }
        }

        query.prepare("DELETE FROM matches WHERE player1 = :username OR player2 = :username");
        query.bindValue(":username", loggedInUser);
        query.exec();

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
    query.prepare("INSERT INTO users (username, password_hash, salt) VALUES (:username, :password_hash, :salt)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", hashed);
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
            "video_path TEXT"
            ")")) {
        QMessageBox::critical(this, "Database Error", "Failed to create matches table: " + query.lastError().text());
        return;
    }

    QSqlQuery pragmaQuery(db);
    bool hasVideoPath = false;
    if (pragmaQuery.exec("PRAGMA table_info(matches)")) {
        while (pragmaQuery.next()) {
            if (pragmaQuery.value(1).toString() == "video_path") {
                hasVideoPath = true;
                break;
            }
        }
    }

    if (!hasVideoPath) {
        QSqlQuery alterQuery(db);
        if (!alterQuery.exec("ALTER TABLE matches ADD COLUMN video_path TEXT")) {
            qDebug() << "Could not add video_path column:" << alterQuery.lastError().text();
        }
    }
}

void TicTacToe::saveMatchResult(const QString &winner, const QString &result) {
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

    char startingPlayer = PLAYER1;
    if (!moveHistory.empty()) {
        startingPlayer = (moveHistory.size() % 2 == 0) ? PLAYER2 : PLAYER1;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO matches "
                  "(player1, player2, winner, result, moves, timestamp, starting_player, video_path) "
                  "VALUES (:player1, :player2, :winner, :result, :moves, :timestamp, :starting_player, :video_path)");
    query.bindValue(":player1", player1Name);
    query.bindValue(":player2", player2Name);
    query.bindValue(":winner", winner);
    query.bindValue(":result", result);
    query.bindValue(":moves", moveSequence);
    query.bindValue(":timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":starting_player", QString(startingPlayer));
    query.bindValue(":video_path", currentVideoPath);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to save match result: " + query.lastError().text());
        return;
    }

    currentMatchId = query.lastInsertId().toInt();
}

void TicTacToe::loadMatchHistory() {
    matchHistoryTable->clearContents();
    matchHistoryTable->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT player1, player2, winner, result, timestamp, video_path FROM matches WHERE player1 = :user OR player2 = :user ORDER BY timestamp DESC");
    query.bindValue(":user", loggedInUser);
    query.exec();

    int row = 0;
    while (query.next()) {
        matchHistoryTable->insertRow(row);
        for (int col = 0; col < 6; ++col) {
            QString text;
            if (col == 0) text = query.value(4).toString();
            else if (col == 5) text = query.value(5).toString().isEmpty() ? "No Video" : "Video Available";
            else text = query.value(col - 1).toString();

            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            matchHistoryTable->setItem(row, col, item);
        }
        ++row;
    }

    stackedWidget->setCurrentIndex(6);
}

void TicTacToe::loadRecordedMatchesScreen() {
    recordedMatchesTable->clearContents();
    recordedMatchesTable->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT id, player1, player2, winner, result, timestamp, video_path FROM matches WHERE player1 = :user OR player2 = :user ORDER BY timestamp DESC");
    query.bindValue(":user", loggedInUser);
    query.exec();

    int row = 0;
    while (query.next()) {
        recordedMatchesTable->insertRow(row);
        for (int col = 0; col < 7; ++col) {
            QString text;
            if (col == 6) {
                text = query.value(6).toString().isEmpty() ? "No Video" : "Video Available";
            } else {
                text = query.value(col).toString();
            }
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            recordedMatchesTable->setItem(row, col, item);
        }
        ++row;
    }

    stackedWidget->setCurrentIndex(7);
}

void TicTacToe::loadRecordedMatch() {
    int row = recordedMatchesTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Match Selected", "Please select a match to replay.");
        return;
    }

    QString matchId = recordedMatchesTable->item(row, 0)->text();
    QString videoPath = recordedMatchesTable->item(row, 6)->text();

    QSqlQuery query;
    query.prepare("SELECT moves, player1, player2, starting_player, video_path FROM matches WHERE id = :id");
    query.bindValue(":id", matchId);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to load match from database.");
        return;
    }

    QString moveStr = query.value(0).toString();
    player1Name = query.value(1).toString();
    player2Name = query.value(2).toString();
    QString startingPlayerStr = query.value(3).toString();
    QString dbVideoPath = query.value(4).toString();
    char startingPlayer = startingPlayerStr.isEmpty() ? PLAYER1 : startingPlayerStr.at(0).toLatin1();

    QStringList moveTokens = moveStr.split(',', Qt::SkipEmptyParts);
    std::vector<int> replayMoves;
    for (const QString &move : moveTokens) {
        replayMoves.push_back(move.toInt());
    }

    board = std::vector<char>(9, EMPTY);
    moveHistory.clear();
    player1Wins = 0;
    player2Wins = 0;
    ties = 0;
    currentPlayer = startingPlayer;

    this->replayMoves = replayMoves;
    this->replayIndex = 0;
    this->replayStartingPlayer = startingPlayer;

    updateBoard();
    updateScoreboard();
    updateStatus();

    if (!dbVideoPath.isEmpty() && QFile::exists(dbVideoPath)) {
        playVideoButton->show();
        videoStatusLabel->setText("Video available for this match");
        currentVideoPath = dbVideoPath;
    } else {
        playVideoButton->hide();
        videoStatusLabel->setText("No video available for this match");
    }

    if (!replayMoves.empty()) {
        replayNextMove();
    }

    QPushButton *surrenderButton = findChild<QPushButton*>("SurrenderButton");
    if (surrenderButton) surrenderButton->hide();

    stackedWidget->setCurrentIndex(2);
}

void TicTacToe::replayNextMove() {
    if (replayIndex >= static_cast<int>(replayMoves.size())) {
        statusLabel->setText("Replay completed");
        return;
    }

    char currentReplayPlayer = (replayIndex % 2 == 0) ? replayStartingPlayer :
                                   (replayStartingPlayer == PLAYER1 ? PLAYER2 : PLAYER1);

    int moveIndex = replayMoves[replayIndex];
    board[moveIndex] = currentReplayPlayer;
    updateBoard();

    bool gameEnded = false;
    if (checkWin(currentReplayPlayer)) {
        if (currentReplayPlayer == PLAYER1) player1Wins++;
        else player2Wins++;
        gameEnded = true;

        QString playerName = (currentReplayPlayer == PLAYER1) ? player1Name : player2Name;
        statusLabel->setText(QString("Replay: %1 (%2) wins this game!")
                                 .arg(playerName).arg(currentReplayPlayer));
    } else if (checkTie()) {
        ties++;
        gameEnded = true;
        statusLabel->setText("Replay: This game is a tie!");
    } else {
        QString playerName = (currentReplayPlayer == PLAYER1) ? player1Name : player2Name;
        statusLabel->setText(QString("Replay: %1 (%2) moves to position %3")
                                 .arg(playerName).arg(currentReplayPlayer).arg(moveIndex + 1));
    }

    updateScoreboard();
    replayIndex++;

    if (gameEnded) {
        QTimer::singleShot(1500, this, [this]() {
            if (player1Wins >= gamesToWin || player2Wins >= gamesToWin ||
                (player1Wins + player2Wins + ties) >= totalGames) {
                QString winner = player1Wins >= gamesToWin ? player1Name : player2Name;
                statusLabel->setText(QString("Replay: %1 wins the series %2-%3!")
                                         .arg(winner)
                                         .arg(qMax(player1Wins, player2Wins))
                                         .arg(qMin(player1Wins, player2Wins)));
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
