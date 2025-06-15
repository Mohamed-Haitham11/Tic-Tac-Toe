#include "tictactoe.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QTimer>
#include <QFont>
#include <QSizePolicy>
#include <random>
#include <algorithm>
#include <limits>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QSqlError>
#include <QDebug>
#include <QFile>
// Define static const members
const char TicTacToe::EMPTY = ' ';
const char TicTacToe::PLAYER1 = 'X';
const char TicTacToe::PLAYER2 = 'O';
// ==================== Constructor ====================
TicTacToe::TicTacToe(QWidget *parent) : QMainWindow(parent) {
    // Initialize game state variables
    board = std::vector<char>(9, EMPTY);
    currentPlayer = PLAYER1;
    mode = 1; // Default to PvP
    difficulty = 1; // Default to Easy
    totalGames = 3;
    gamesToWin = 2;
    player1Wins = 0;
    player2Wins = 0;
    ties = 0;
    scoreboardVisible = false;
    firstMoveMade = false;
    guestMode = false;
    selectedTheme = "Light";
    replayIndex = 0;
    isRecording = false;
    currentMatchId = 0;

    setupUI();
    connectToDatabase();
    createTablesIfNeeded();
    setupVideoRecording();
    setupVideoPlayback();
    createVideoDirectory();
    applyStyleSheet();
    resetGame();
}

// ==================== Mode Selection ====================
void TicTacToe::setPlayerVsPlayer() {
    mode = 1;
    stackedWidget->setCurrentIndex(5);
}

void TicTacToe::setPlayerVsAI() {
    mode = 2;
    stackedWidget->setCurrentIndex(3);
}

void TicTacToe::setDifficultyEasy() {
    difficulty = 1;
    stackedWidget->setCurrentIndex(5);
}

void TicTacToe::setDifficultyMedium() {
    difficulty = 2;
    stackedWidget->setCurrentIndex(5);
}

void TicTacToe::setDifficultyHard() {
    difficulty = 3;
    stackedWidget->setCurrentIndex(5);
}

// ==================== Game Settings and Start ====================
void TicTacToe::applyGameSettings() {
    totalGames = totalGamesSpinBox->value();
    gamesToWin = gamesToWinSpinBox->value();

    if (gamesToWin > totalGames) {
        QMessageBox::warning(this, "Invalid Settings", "Games to win cannot be greater than total games!");
        return;
    }

    if (mode == 1) {
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
        // In AI mode: AI is always X (PLAYER1), Human is always O (PLAYER2)
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
    stackedWidget->setCurrentIndex(2);
    resetGame();
}

// ==================== Core Game Logic ====================
void TicTacToe::resetGame() {
    if (player1Wins >= gamesToWin || player2Wins >= gamesToWin ||
        (player1Wins + player2Wins + ties) >= totalGames) {
        backToModeSelection();
        return;
    }

    board = std::vector<char>(9, EMPTY);
    moveHistory.clear();
    firstMoveMade = false; // Reset first move flag

    // Randomly decide who starts (50% chance for AI or player)
    std::random_device rd;
    std::mt19937 gen(rd());
    if (gen() % 2 == 0) {
        currentPlayer = PLAYER1; // AI starts
        if (mode == 2) {
            QTimer::singleShot(500, this, [this]() { makeAIMove(); });
        }
    } else {
        currentPlayer = PLAYER2; // Player starts
    }

    // Clear video path for new game (only if not recording)
    if (!isRecording) {
        currentVideoPath.clear();
        currentMatchId = 0;
    }

    scoreboardVisible = false;
    scoreLabel->setVisible(scoreboardVisible);
    if (scoreboardToggleButton) {
        scoreboardToggleButton->setText("Show Scoreboard");
    }

    updateScoreboard();
    updateBoard();
    updateStatus();

    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
        if (button) {
            button->setText("");
            button->setEnabled(true);
        }
    }
}

void TicTacToe::makeMove(int index) {
    if (board[index] != EMPTY) return;

    board[index] = currentPlayer;
    moveHistory.push_back(index);
    updateBoard();

    if (checkWin(currentPlayer)) {
        if (currentPlayer == PLAYER1) player1Wins++;
        else player2Wins++;
        updateScoreboard();

        if (player1Wins >= gamesToWin || player2Wins >= gamesToWin) {
            QString winner = player1Wins >= gamesToWin ? player1Name : player2Name;
            gameOver(QString("%1 wins the series %2-%3!")
                         .arg(winner)
                         .arg(qMax(player1Wins, player2Wins))
                         .arg(qMin(player1Wins, player2Wins)), true);
            return;
        }

        if ((player1Wins + player2Wins + ties) >= totalGames) {
            if (player1Wins > player2Wins) {
                gameOver(QString("%1 wins the series %2-%3!")
                             .arg(player1Name).arg(player1Wins).arg(player2Wins), true);
            } else if (player2Wins > player1Wins) {
                gameOver(QString("%1 wins the series %2-%3!")
                             .arg(player2Name).arg(player2Wins).arg(player1Wins), true);
            } else {
                gameOver("Series ended in a tie!", true);
            }
            return;
        }

        gameOver(QString("%1 (%2) wins!").arg(currentPlayer == PLAYER1 ? player1Name : player2Name).arg(currentPlayer));
        return;
    }

    if (checkTie()) {
        ties++;
        updateScoreboard();

        if ((player1Wins + player2Wins + ties) >= totalGames) {
            if (player1Wins > player2Wins) {
                gameOver(QString("%1 wins the series %2-%3!")
                             .arg(player1Name).arg(player1Wins).arg(player2Wins), true);
            } else if (player2Wins > player1Wins) {
                gameOver(QString("%1 wins the series %2-%3!")
                             .arg(player2Name).arg(player2Wins).arg(player1Wins), true);
            } else {
                gameOver("Series ended in a tie!", true);
            }
            return;
        }

        gameOver("It's a tie!");
        return;
    }

    // Switch players
    currentPlayer = (currentPlayer == PLAYER1) ? PLAYER2 : PLAYER1;
    updateStatus();

    // If it's the AI's turn in PvAI mode, schedule the AI move
    if (mode == 2 && currentPlayer == PLAYER1) {
        QTimer::singleShot(500, this, [this]() { makeAIMove(); });
    }
}

// ==================== AI Logic ====================
void TicTacToe::makeAIMove() {
    int move = -1;

    if (difficulty == 1) {
        move = easyMove();
    } else if (difficulty == 2) {
        move = mediumMove();
    } else if (difficulty == 3) {
        move = hardMove();
    }

    if (move != -1) {
        makeMove(move);
    }
}
int TicTacToe::easyMove() {
    std::vector<int> availableMoves;
    std::vector<int> moveScores;

    // Analyze each possible move
    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            availableMoves.push_back(i);
            std::vector<char> tempBoard = board; // Create a copy
            tempBoard[i] = PLAYER1; // Try the move for the AI
            int score = minimax(tempBoard, false); // Get the minimax score
            moveScores.push_back(score);
        }
    }

    // For easy difficulty: Avoid moves with score = 1 (winning moves for AI)
    std::vector<int> safeMoves;
    for (size_t i = 0; i < availableMoves.size(); i++) {
        if (moveScores[i] != 1) {
            safeMoves.push_back(availableMoves[i]);
        }
    }

    // Prefer moves with score = -1 (blocking moves)
    for (size_t i = 0; i < safeMoves.size(); i++) {
        int index = std::find(availableMoves.begin(), availableMoves.end(), safeMoves[i]) - availableMoves.begin();
        if (moveScores[index] == -1) {
            return safeMoves[i];
        }
    }

    // If not found, choose moves with score = 0
    for (size_t i = 0; i < safeMoves.size(); i++) {
        int index = std::find(availableMoves.begin(), availableMoves.end(), safeMoves[i]) - availableMoves.begin();
        if (moveScores[index] == 0) {
            return safeMoves[i];
        }
    }

    // Otherwise, pick a fallback move
    return safeMoves.empty() ? availableMoves[0] : safeMoves[0];
}

int TicTacToe::mediumMove() {
    std::vector<int> availableMoves;
    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            availableMoves.push_back(i);
        }
    }

    // Randomly choose one of the available moves
    if (!availableMoves.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, availableMoves.size() - 1);
        return availableMoves[dis(gen)];
    }
    return -1; // No move available
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
    // Create a temporary TicTacToe instance to use checkWin and checkTie
    std::vector<char> originalBoard = board;
    board = tempBoard;

    bool aiWins = checkWin(PLAYER1);
    bool playerWins = checkWin(PLAYER2);
    bool tie = checkTie();

    board = originalBoard; // Restore original board

    if (aiWins) return 1;   // AI wins
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

// ==================== Win / Tie Check ====================
void TicTacToe::gameOver(const QString &message, bool seriesOver) {
    statusLabel->setText(message);
    if (!scoreboardVisible)
        toggleScoreboard();
    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
        if (button) button->setEnabled(false);
    }

    // Save EVERY game result, not just series results
    if (!guestMode) {
        QString winner;
        QString result;

        // Determine the winner of THIS individual game
        if (message.contains("wins!")) {
            // Individual game win
            if (message.contains(player1Name)) {
                winner = player1Name;
                result = "Game Win";
            } else {
                winner = player2Name;
                result = "Game Win";
            }
        } else if (message.contains("tie")) {
            // Individual game tie
            winner = "-";
            result = "Game Tie";
        }

        // Save this individual game result
        if (!winner.isEmpty()) {
            saveMatchResult(winner, result);
        }

        // If this is a series conclusion, save that too
        if (seriesOver) {
            QString seriesWinner;
            QString seriesResult;

            if (player1Wins > player2Wins) {
                seriesWinner = player1Name;
                seriesResult = "Series Win";
            } else if (player2Wins > player1Wins) {
                seriesWinner = player2Name;
                seriesResult = "Series Win";
            } else {
                seriesWinner = "-";
                seriesResult = "Series Tie";
            }

            // Save series result as a separate entry
            saveMatchResult(seriesWinner, seriesResult);
        }

        // Stop recording only when series is over
        if (isRecording && seriesOver) {
            stopVideoRecording();
        }
    }

    if (seriesOver)
        QTimer::singleShot(3000, this, &TicTacToe::backToModeSelection);
    else
        QTimer::singleShot(2000, this, &TicTacToe::resetGame);
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

void TicTacToe::toggleScoreboard() {
    scoreboardVisible = !scoreboardVisible;
    scoreLabel->setVisible(scoreboardVisible);
    if (scoreboardToggleButton) {
        scoreboardToggleButton->setText(scoreboardVisible ? "Hide Scoreboard" : "Show Scoreboard");
    }
}

// ==================== Button Handler ====================
void TicTacToe::handleButtonClick(int index) {
    // Prevent move if the cell is not empty or in PvAI mode during AI's turn
    if (board[index] != EMPTY || (mode == 2 && currentPlayer == PLAYER1)) return;
    makeMove(index);
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

    if (isRecording) {
        stopVideoRecording();
    }

    QMessageBox::information(this, "Surrender", QString::fromUtf8(u8"🏳️ %1 wins by surrender!").arg(winner));
    backToModeSelection();
}

void TicTacToe::backToModeSelection() {
    if (isRecording) {
        stopVideoRecording();
    }

    player1Wins = 0;
    player2Wins = 0;
    ties = 0;
    updateScoreboard();
    stackedWidget->setCurrentIndex(1);
}

// ==================== Video Recording Functions ====================
void TicTacToe::setupVideoRecording() {
    // Create components but don't connect them yet
    mediaRecorder = new QMediaRecorder(this);
    captureSession = new QMediaCaptureSession(this);
    screenCapture = new QScreenCapture(this);

    // DO NOT set up the capture session yet - only when recording starts
    // captureSession->setScreenCapture(screenCapture);  // Remove this line
    // captureSession->setRecorder(mediaRecorder);       // Remove this line

    mediaRecorder->setQuality(QMediaRecorder::HighQuality);
    mediaRecorder->setVideoFrameRate(30);

    connect(mediaRecorder, &QMediaRecorder::recorderStateChanged, this, [this](QMediaRecorder::RecorderState state) {
        if (state == QMediaRecorder::StoppedState) {
            onRecordingFinished();
        }
    });

    connect(mediaRecorder, &QMediaRecorder::errorOccurred, this, &TicTacToe::onRecordingError);

    // Explicitly ensure everything is stopped
    isRecording = false;

    // Make sure nothing is active
    if (screenCapture->isActive()) {
        screenCapture->stop();
    }

    if (mediaRecorder->recorderState() != QMediaRecorder::StoppedState) {
        mediaRecorder->stop();
    }

    qDebug() << "Video recording setup completed - NOT recording";
}

void TicTacToe::setupVideoPlayback() {
    videoPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    videoWidget = new QVideoWidget();

    videoPlayer->setVideoOutput(videoWidget);
    videoPlayer->setAudioOutput(audioOutput);

    connect(videoPlayer, &QMediaPlayer::positionChanged, this, &TicTacToe::onVideoPositionChanged);
    connect(videoPlayer, &QMediaPlayer::durationChanged, this, &TicTacToe::onVideoDurationChanged);
}

void TicTacToe::createVideoDirectory() {
    videoDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/TicTacToe_Videos";
    QDir dir;
    if (!dir.exists(videoDirectory)) {
        dir.mkpath(videoDirectory);
    }
}

QString TicTacToe::generateVideoFileName() {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = QString("TicTacToe_Match_%1_%2_vs_%3.mp4")
                           .arg(timestamp)
                           .arg(player1Name.replace(" ", "_"))
                           .arg(player2Name.replace(" ", "_"));
    return QDir(videoDirectory).absoluteFilePath(filename);
}

void TicTacToe::startVideoRecording() {
    if (isRecording) {
        qDebug() << "Already recording, ignoring start request";
        return;
    }

    qDebug() << "Starting video recording...";

    // Set up the capture session only when starting recording
    captureSession->setScreenCapture(screenCapture);
    captureSession->setRecorder(mediaRecorder);

    currentVideoPath = generateVideoFileName();
    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(currentVideoPath));

    // Start screen capture first
    screenCapture->start();

    // Then start recording
    mediaRecorder->record();

    isRecording = true;
    recordButton->setText("Stop Recording");
    recordingStatusLabel->setText("🔴 Recording...");
    recordingStatusLabel->setStyleSheet("color: red; font-weight: bold;");

    qDebug() << "Recording started to:" << currentVideoPath;
}

void TicTacToe::stopVideoRecording() {
    if (!isRecording) {
        qDebug() << "Not recording, ignoring stop request";
        return;
    }

    qDebug() << "Stopping video recording...";

    // Stop recording first
    if (mediaRecorder->recorderState() != QMediaRecorder::StoppedState) {
        mediaRecorder->stop();
    }

    // Stop screen capture
    if (screenCapture->isActive()) {
        screenCapture->stop();
    }

    // Disconnect the capture session to prevent auto-restart
    captureSession->setScreenCapture(nullptr);
    captureSession->setRecorder(nullptr);

    isRecording = false;
    recordButton->setText("Start Recording");
    recordingStatusLabel->setText("⚫ Not Recording");
    recordingStatusLabel->setStyleSheet("color: gray;");

    qDebug() << "Recording stopped";
}
void TicTacToe::onRecordingFinished() {
    if (!currentVideoPath.isEmpty() && currentMatchId > 0) {
        updateVideoDatabase(currentMatchId, currentVideoPath);
    }
    recordingStatusLabel->setText("✅ Recording Saved");
    recordingStatusLabel->setStyleSheet("color: green; font-weight: bold;");

    QTimer::singleShot(3000, this, [this]() {
        recordingStatusLabel->setText("⚫ Not Recording");
        recordingStatusLabel->setStyleSheet("color: gray;");
    });
}

void TicTacToe::onRecordingError() {
    QMessageBox::warning(this, "Recording Error", "Failed to record video: " + mediaRecorder->errorString());
    isRecording = false;
    recordButton->setText("Start Recording");
    recordingStatusLabel->setText("❌ Recording Failed");
    recordingStatusLabel->setStyleSheet("color: red;");
}

void TicTacToe::updateVideoDatabase(int matchId, const QString &videoPath) {
    QSqlQuery query;
    query.prepare("UPDATE matches SET video_path = :video_path WHERE id = :id");
    query.bindValue(":video_path", videoPath);
    query.bindValue(":id", matchId);

    if (!query.exec()) {
        qDebug() << "Failed to update video path in database:" << query.lastError().text();
    }
}

// ==================== Video Playback Functions ====================
void TicTacToe::playRecordedVideo() {
    if (currentVideoPath.isEmpty() || !QFile::exists(currentVideoPath)) {
        QMessageBox::warning(this, "Video Error", "Video file not found.");
        return;
    }

    videoPlayer->setSource(QUrl::fromLocalFile(currentVideoPath));
    videoWidget->show();
    videoWidget->resize(800, 600);
    videoPlayer->play();

    playVideoButton->setText("Pause Video");
    stopVideoButton->show();
}

void TicTacToe::stopVideoPlayback() {
    videoPlayer->stop();
    videoWidget->hide();
    playVideoButton->setText("Play Video");
    stopVideoButton->hide();
    videoStatusLabel->setText("Video stopped");
}

void TicTacToe::onVideoPositionChanged(qint64 position) {
    if (videoPlayer->duration() > 0) {
        int progress = (position * 100) / videoPlayer->duration();
        videoStatusLabel->setText(QString("Video: %1%").arg(progress));
        if (videoProgressSlider) {
            videoProgressSlider->setValue(progress);
        }
    }
}

void TicTacToe::onVideoDurationChanged(qint64 duration) {
    Q_UNUSED(duration)
}
