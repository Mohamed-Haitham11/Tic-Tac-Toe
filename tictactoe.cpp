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
#include <QDateTime> // Added
#include <QSqlError> // Added
#include <QDebug>    // Added

// ==================== Constructor ====================
TicTacToe::TicTacToe(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    connectToDatabase();
    createTablesIfNeeded();
    applyStyleSheet();
    resetGame();
}

// ==================== UI Setup ====================
// (setupUI() - Full UI Setup for Login, Mode selection, Difficulty selection, Player 2 name, Settings, Game Board)
void TicTacToe::setupUI() {
    stackedWidget = new QStackedWidget(this);

    // === Login Screen (Index 0) ===
    QWidget *loginWidget = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginWidget);

    QHBoxLayout *logoRowLayout = new QHBoxLayout();
    QLabel *logoLabel = new QLabel(this);
    logoLabel->setFixedSize(100, 100);
    logoLabel->setObjectName("themeLogo");
    logoRowLayout->addWidget(logoLabel);
    logoRowLayout->addStretch();
    loginLayout->addLayout(logoRowLayout);

    QLabel *titleLabel = new QLabel("Tic Tac Toe");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(36);
    titleLabel->setFont(titleFont);
    loginLayout->addWidget(titleLabel);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Username");
    usernameEdit->setFixedHeight(40);
    usernameEdit->setFont(QFont("Arial", 14));
    usernameEdit->setObjectName("usernameLoginEdit"); // Consistent ID for stylesheet
    loginLayout->addWidget(usernameEdit);

    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(40);
    passwordEdit->setFont(QFont("Arial", 14));
    passwordEdit->setObjectName("passwordLoginEdit"); // Consistent ID for stylesheet
    loginLayout->addWidget(passwordEdit);

    QPushButton *loginButton = new QPushButton("Login");
    loginButton->setFixedHeight(50);
    loginButton->setFont(QFont("Arial", 16));
    loginLayout->addWidget(loginButton);
    connect(loginButton, &QPushButton::clicked, this, &TicTacToe::handleLogin);

    QPushButton *registerButton = new QPushButton("Register");
    registerButton->setFixedHeight(50);
    registerButton->setFont(QFont("Arial", 16));
    loginLayout->addWidget(registerButton);
    connect(registerButton, &QPushButton::clicked, this, &TicTacToe::registerAccount);

    QPushButton *guestButton = new QPushButton("Play as Guest");
    guestButton->setFixedHeight(50);
    guestButton->setFont(QFont("Arial", 16));
    loginLayout->addWidget(guestButton);
    connect(guestButton, &QPushButton::clicked, this, &TicTacToe::guestLogin);

    loginLayout->addStretch();
    stackedWidget->addWidget(loginWidget);

    // === Mode Selection Screen (Index 1) ===
    QWidget *modeSelectWidget = new QWidget();
    QVBoxLayout *modeSelectLayout = new QVBoxLayout(modeSelectWidget);
    QLabel *modeTitleLabel = new QLabel("Select Mode");
    modeTitleLabel->setAlignment(Qt::AlignCenter);
    modeTitleLabel.setFont(titleFont);
    modeSelectLayout->addWidget(modeTitleLabel);

    QPushButton *pvpButton = new QPushButton("Player vs. Player");
    pvpButton->setFixedHeight(60);
    pvpButton->setFont(QFont("Arial", 18));
    modeSelectLayout->addWidget(pvpButton);
    connect(pvpButton, &QPushButton::clicked, this, &TicTacToe::setPlayerVsPlayer);

    QPushButton *pvaiButton = new QPushButton("Player vs. AI");
    pvaiButton->setFixedHeight(60);
    pvaiButton->setFont(QFont("Arial", 18));
    modeSelectLayout->addWidget(pvaiButton);
    connect(pvaiButton, &QPushButton::clicked, this, &TicTacToe::setPlayerVsAI);

    QPushButton *logoutButton = new QPushButton("Logout");
    logoutButton->setFixedHeight(50);
    logoutButton->setFont(QFont("Arial", 16));
    modeSelectLayout->addWidget(logoutButton);
    connect(logoutButton, &QPushButton::clicked, this, &TicTacToe::logout);

    QPushButton *matchHistoryButton = new QPushButton("Match History");
    matchHistoryButton->setFixedHeight(50);
    matchHistoryButton->setFont(QFont("Arial", 16));
    modeSelectLayout->addWidget(matchHistoryButton);
    // Connect matchHistoryButton to a new slot for showing match history
    connect(matchHistoryButton, &QPushButton::clicked, this, [this]() {
        loadMatchHistory();
        stackedWidget->setCurrentIndex(6); // Switch to match history screen
    });

    modeSelectLayout->addStretch();
    stackedWidget->addWidget(modeSelectWidget);

    // === AI Difficulty Screen (Index 2) ===
    QWidget *difficultyWidget = new QWidget();
    QVBoxLayout *difficultyLayout = new QVBoxLayout(difficultyWidget);
    QLabel *difficultyTitleLabel = new QLabel("Select AI Difficulty");
    difficultyTitleLabel->setAlignment(Qt::AlignCenter);
    difficultyTitleLabel.setFont(titleFont);
    difficultyLayout->addWidget(difficultyTitleLabel);

    QPushButton *easyButton = new QPushButton("Easy");
    easyButton->setFixedHeight(60);
    easyButton->setFont(QFont("Arial", 18));
    difficultyLayout->addWidget(easyButton);
    connect(easyButton, &QPushButton::clicked, this, &TicTacToe::setDifficultyEasy);

    QPushButton *mediumButton = new QPushButton("Medium");
    mediumButton->setFixedHeight(60);
    mediumButton->setFont(QFont("Arial", 18));
    difficultyLayout->addWidget(mediumButton);
    connect(mediumButton, &QPushButton::clicked, this, &TicTacToe::setDifficultyMedium);

    QPushButton *hardButton = new QPushButton("Hard");
    hardButton->setFixedHeight(60);
    hardButton->setFont(QFont("Arial", 18));
    difficultyLayout->addWidget(hardButton);
    connect(hardButton, &QPushButton::clicked, this, &TicTacToe::setDifficultyHard);

    QPushButton *backFromDifficultyButton = new QPushButton("Back");
    backFromDifficultyButton->setFixedHeight(50);
    backFromDifficultyButton->setFont(QFont("Arial", 16));
    difficultyLayout->addWidget(backFromDifficultyButton);
    connect(backFromDifficultyButton, &QPushButton::clicked, this, &TicTacToe::backToModeSelection);

    difficultyLayout->addStretch();
    stackedWidget->addWidget(difficultyWidget);

    // === Player 2 Name Entry (Index 3) ===
    QWidget *player2NameWidget = new QWidget();
    QVBoxLayout *player2NameLayout = new QVBoxLayout(player2NameWidget);
    QLabel *player2NameTitle = new QLabel("Enter Player 2 Name");
    player2NameTitle->setAlignment(Qt::AlignCenter);
    player2NameTitle.setFont(titleFont);
    player2NameLayout->addWidget(player2NameTitle);

    player2NameEdit = new QLineEdit();
    player2NameEdit->setPlaceholderText("Player 2 Name");
    player2NameEdit->setFixedHeight(40);
    player2NameEdit->setFont(QFont("Arial", 14));
    player2NameLayout->addWidget(player2NameEdit);

    QPushButton *startPvPButton = new QPushButton("Start Game");
    startPvPButton->setFixedHeight(50);
    startPvPButton->setFont(QFont("Arial", 16));
    player2NameLayout->addWidget(startPvPButton);
    connect(startPvPButton, &QPushButton::clicked, this, &TicTacToe::startPvPWithNames);

    QPushButton *backFromPlayer2NameButton = new QPushButton("Back");
    backFromPlayer2NameButton->setFixedHeight(50);
    backFromPlayer2NameButton->setFont(QFont("Arial", 16));
    player2NameLayout->addWidget(backFromPlayer2NameButton);
    connect(backFromPlayer2NameButton, &QPushButton::clicked, this, &TicTacToe::backToModeSelection);

    player2NameLayout->addStretch();
    stackedWidget->addWidget(player2NameWidget);

    // === Settings Screen (Index 4) ===
    QWidget *settingsWidget = new QWidget();
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsWidget);
    QLabel *settingsTitleLabel = new QLabel("Game Settings");
    settingsTitleLabel->setAlignment(Qt::AlignCenter);
    settingsTitleLabel.setFont(titleFont);
    settingsLayout->addWidget(settingsTitleLabel);

    QHBoxLayout *gamesRowLayout = new QHBoxLayout();
    QLabel *totalGamesLabel = new QLabel("Total Games in Series:");
    totalGamesLabel->setFont(QFont("Arial", 14));
    gamesRowLayout->addWidget(totalGamesLabel);
    totalGamesSpinBox = new QSpinBox();
    totalGamesSpinBox->setRange(1, 10);
    totalGamesSpinBox->setValue(totalGames);
    gamesRowLayout->addWidget(totalGamesSpinBox);
    settingsLayout->addLayout(gamesRowLayout);

    QHBoxLayout *winsRowLayout = new QHBoxLayout();
    QLabel *gamesToWinLabel = new QLabel("Games to Win Series:");
    gamesToWinLabel->setFont(QFont("Arial", 14));
    winsRowLayout->addWidget(gamesToWinLabel);
    gamesToWinSpinBox = new QSpinBox();
    gamesToWinSpinBox->setRange(1, 10);
    gamesToWinSpinBox->setValue(gamesToWin);
    winsRowLayout->addWidget(gamesToWinSpinBox);
    settingsLayout->addLayout(winsRowLayout);

    QPushButton *applySettingsButton = new QPushButton("Apply Settings");
    applySettingsButton->setFixedHeight(50);
    applySettingsButton->setFont(QFont("Arial", 16));
    settingsLayout->addWidget(applySettingsButton);
    connect(applySettingsButton, &QPushButton::clicked, this, &TicTacToe::applyGameSettings);

    nightModeButton = new QPushButton("Toggle Night Mode");
    nightModeButton->setFixedHeight(50);
    nightModeButton->setFont(QFont("Arial", 16));
    settingsLayout->addWidget(nightModeButton);
    connect(nightModeButton, &QPushButton::clicked, this, &TicTacToe::toggleNightMode);

    scoreboardToggleButton = new QPushButton("Toggle Scoreboard");
    scoreboardToggleButton->setFixedHeight(50);
    scoreboardToggleButton->setFont(QFont("Arial", 16));
    settingsLayout->addWidget(scoreboardToggleButton);
    connect(scoreboardToggleButton, &QPushButton::clicked, this, &TicTacToe::toggleScoreboard);

    // Theme Selector
    QHBoxLayout *themeLayout = new QHBoxLayout();
    QLabel *themeLabel = new QLabel("Select Theme:");
    themeLabel->setFont(QFont("Arial", 14));
    themeLayout->addWidget(themeLabel);
    themeSelector = new QComboBox();
    themeSelector->addItem("Light");
    themeSelector->addItem("Dark");
    themeSelector->addItem("Red");
    themeSelector->addItem("Blue");
    themeSelector->addItem("Green");
    themeLayout->addWidget(themeSelector);
    settingsLayout->addLayout(themeLayout);
    connect(themeSelector, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
            this, &TicTacToe::applyStyleSheet);


    QPushButton *backFromSettingsButton = new QPushButton("Back");
    backFromSettingsButton->setFixedHeight(50);
    backFromSettingsButton->setFont(QFont("Arial", 16));
    settingsLayout->addWidget(backFromSettingsButton);
    connect(backFromSettingsButton, &QPushButton::clicked, this, &TicTacToe::backToModeSelection);

    settingsLayout->addStretch();
    stackedWidget->addWidget(settingsWidget);

    // === Game Board Screen (Index 5) ===
    QWidget *gameWidget = new QWidget();
    QVBoxLayout *gameLayout = new QVBoxLayout(gameWidget);

    scoreLabel = new QLabel("Score: Player 1 (X) 0 - 0 Player 2 (O) | Ties: 0");
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setFont(QFont("Arial", 16));
    gameLayout->addWidget(scoreLabel);

    statusLabel = new QLabel("Player X's Turn");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFont(QFont("Arial", 24, QFont::Bold));
    gameLayout->addWidget(statusLabel);

    QGridLayout *boardLayout = new QGridLayout();
    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(false);

    for (int i = 0; i < 9; ++i) {
        QPushButton *button = new QPushButton("");
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setFont(QFont("Arial", 48, QFont::Bold));
        button->setFixedSize(100, 100);
        buttonGroup->addButton(button, i);
        boardLayout->addWidget(button, i / 3, i % 3);
    }
    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &TicTacToe::handleButtonClick);
    gameLayout->addLayout(boardLayout);

    QPushButton *resetButton = new QPushButton("Reset Game");
    resetButton->setFixedHeight(50);
    resetButton->setFont(QFont("Arial", 16));
    gameLayout->addWidget(resetButton);
    connect(resetButton, &QPushButton::clicked, this, &TicTacToe::resetGame);

    QPushButton *surrenderButton = new QPushButton("Surrender");
    surrenderButton->setFixedHeight(50);
    surrenderButton->setFont(QFont("Arial", 16));
    gameLayout->addWidget(surrenderButton);
    connect(surrenderButton, &QPushButton::clicked, this, &TicTacToe::handleSurrender); // Assuming handleSurrender exists

    QPushButton *backToModeButton = new QPushButton("Back to Mode Selection");
    backToModeButton->setFixedHeight(50);
    backToModeButton->setFont(QFont("Arial", 16));
    gameLayout->addWidget(backToModeButton);
    connect(backToModeButton, &QPushButton::clicked, this, &TicTacToe::backToModeSelection);

    stackedWidget->addWidget(gameWidget);

    // === Match History Screen (Index 6) ===
    QWidget *matchHistoryWidget = new QWidget();
    QVBoxLayout *matchHistoryLayout = new QVBoxLayout(matchHistoryWidget);
    QLabel *historyTitleLabel = new QLabel("Match History");
    historyTitleLabel->setAlignment(Qt::AlignCenter);
    historyTitleLabel.setFont(titleFont);
    matchHistoryLayout->addWidget(historyTitleLabel);

    matchHistoryTable = new QTableWidget();
    matchHistoryTable->setColumnCount(4);
    matchHistoryTable->setHorizontalHeaderLabels({"Timestamp", "User", "Winner", "Result"});
    matchHistoryTable->horizontalHeader()->setStretchLastSection(true);
    matchHistoryTable->verticalHeader()->setVisible(false); // Hide row numbers
    matchHistoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only
    matchHistoryLayout->addWidget(matchHistoryTable);

    QPushButton *backFromHistoryButton = new QPushButton("Back");
    backFromHistoryButton->setFixedHeight(50);
    backFromHistoryButton->setFont(QFont("Arial", 16));
    matchHistoryLayout->addWidget(backFromHistoryButton);
    connect(backFromHistoryButton, &QPushButton::clicked, this, [this]() {
        if (guestMode) {
            stackedWidget->setCurrentIndex(0); // Back to login for guests
        } else {
            stackedWidget->setCurrentIndex(1); // Back to mode selection for logged-in users
        }
    });

    matchHistoryLayout->addStretch();
    stackedWidget->addWidget(matchHistoryWidget);


    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentIndex(0); // Start at login screen
}

// ==================== Game Logic ====================
void TicTacToe::resetGame() {
    board.assign(9, EMPTY);
    currentPlayer = PLAYER1;
    updateBoard();
    updateStatus();
    updateScoreboard();
    buttonGroup->setExclusive(false); // Enable all buttons
}

void TicTacToe::updateBoard() {
    for (int i = 0; i < 9; ++i) {
        QPushButton *button = qobject_cast<QPushButton*>(buttonGroup->button(i));
        if (button) {
            button->setText(QString(board[i]));
            button->setEnabled(board[i] == EMPTY); // Enable only empty cells
        }
    }
}

void TicTacToe::updateStatus() {
    if (checkWin(PLAYER1)) {
        statusLabel->setText("Player X Wins!");
        gameOver("Player X");
    } else if (checkWin(PLAYER2)) {
        statusLabel->setText("Player O Wins!");
        gameOver("Player O");
    } else if (checkTie()) {
        statusLabel->setText("It's a Tie!");
        gameOver("Tie");
    } else {
        statusLabel->setText(QString("Player %1's Turn").arg(currentPlayer == PLAYER1 ? "X" : "O"));
    }
}

void TicTacToe::makeMove(int index) {
    if (board[index] == EMPTY) {
        board[index] = currentPlayer;
        updateBoard();
        if (checkWin(currentPlayer) || checkTie()) {
            updateStatus();
            return;
        }

        currentPlayer = (currentPlayer == PLAYER1) ? PLAYER2 : PLAYER1;
        updateStatus();

        if (mode == 2 && currentPlayer == PLAYER2) { // AI's turn
            QTimer::singleShot(500, this, &TicTacToe::makeAIMove);
        }
    }
}

void TicTacToe::gameOver(const QString &message, bool seriesOver) {
    if (message == "Player X") {
        player1Wins++;
    } else if (message == "Player O") {
        player2Wins++;
    } else if (message == "Tie") {
        ties++;
    }

    saveMatchResult(loggedInUser, message); // Save match result
    updateScoreboard();

    if (!seriesOver && (player1Wins >= gamesToWin || player2Wins >= gamesToWin || (player1Wins + player2Wins + ties) == totalGames)) {
        QString seriesWinnerMessage;
        bool finalSeriesOver = false;

        if (player1Wins >= gamesToWin) {
            seriesWinnerMessage = "Player X wins the series!";
            finalSeriesOver = true;
        } else if (player2Wins >= gamesToWin) {
            seriesWinnerMessage = "Player O wins the series!";
            finalSeriesOver = true;
        } else if ((player1Wins + player2Wins + ties) == totalGames) {
            seriesWinnerMessage = "Series over: All games played!";
            finalSeriesOver = true;
        }

        if (finalSeriesOver) {
            QMessageBox::information(this, "Series Over", seriesWinnerMessage);
            player1Wins = 0;
            player2Wins = 0;
            ties = 0;
            resetGame();
        } else {
            QMessageBox::information(this, "Game Over", message + "! Next game.");
            resetGame();
        }
    } else {
        QMessageBox::information(this, "Game Over", message + "! Next game.");
        resetGame();
    }
}

void TicTacToe::makeAIMove() {
    if (mode != 2) return; // Only for PvAI mode

    if (difficulty == 1) { // Easy
        easyMove();
    } else if (difficulty == 2) { // Medium
        mediumMove();
    } else if (difficulty == 3) { // Hard
        hardMove();
    }
}

void TicTacToe::easyMove() {
    std::vector<int> emptyCells;
    for (int i = 0; i < 9; ++i) {
        if (board[i] == EMPTY) {
            emptyCells.push_back(i);
        }
    }
    if (!emptyCells.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, emptyCells.size() - 1);
        int randomIndex = distrib(gen);
        makeMove(emptyCells[randomIndex]);
    }
}

void TicTacToe::mediumMove() {
    // Check for a winning move for AI
    for (int i = 0; i < 9; ++i) {
        if (board[i] == EMPTY) {
            board[i] = PLAYER2;
            if (checkWin(PLAYER2)) {
                makeMove(i);
                return;
            }
            board[i] = EMPTY; // backtrack
        }
    }

    // Check for a winning move for Player 1 (block)
    for (int i = 0; i < 9; ++i) {
        if (board[i] == EMPTY) {
            board[i] = PLAYER1;
            if (checkWin(PLAYER1)) {
                makeMove(i);
                return;
            }
            board[i] = EMPTY; // backtrack
        }
    }

    // If no immediate win or block, make a random easy move
    easyMove();
}

void TicTacToe::hardMove() {
    int bestScore = std::numeric_limits<int>::min();
    int bestMove = -1;

    for (int i = 0; i < 9; ++i) {
        if (board[i] == EMPTY) {
            board[i] = PLAYER2; // AI's turn (Maximizing player)
            int score = minimax(0, false); // Call minimax for player 1 (Minimizing player)
            board[i] = EMPTY; // backtrack

            if (score > bestScore) {
                bestScore = score;
                bestMove = i;
            }
        }
    }
    if (bestMove != -1) {
        makeMove(bestMove);
    }
}

int TicTacToe::minimax(int depth, bool isMaximizing) {
    if (checkWin(PLAYER2)) { // AI wins
        return 10 - depth;
    } else if (checkWin(PLAYER1)) { // Player 1 wins
        return depth - 10;
    } else if (checkTie()) { // Tie
        return 0;
    }

    // If depth limit is reached, return 0 (neutral score)
    // This is a common heuristic to prevent infinite recursion in complex games or limit search depth.
    // Adjust max_depth as needed.
    const int MAX_DEPTH = 6; // Example depth limit
    if (depth >= MAX_DEPTH) {
        return 0;
    }

    if (isMaximizing) { // AI's turn
        int bestScore = std::numeric_limits<int>::min();
        for (int i = 0; i < 9; ++i) {
            if (board[i] == EMPTY) {
                board[i] = PLAYER2;
                int score = minimax(depth + 1, false);
                board[i] = EMPTY;
                bestScore = std::max(bestScore, score);
            }
        }
        return bestScore;
    } else { // Player 1's turn
        int bestScore = std::numeric_limits<int>::max();
        for (int i = 0; i < 9; ++i) {
            if (board[i] == EMPTY) {
                board[i] = PLAYER1;
                int score = minimax(depth + 1, true);
                board[i] = EMPTY;
                bestScore = std::min(bestScore, score);
            }
        }
        return bestScore;
    }
}

bool TicTacToe::checkWin(char player) {
    // Check rows
    for (int i = 0; i < 9; i += 3) {
        if (board[i] == player && board[i+1] == player && board[i+2] == player) return true;
    }
    // Check columns
    for (int i = 0; i < 3; ++i) {
        if (board[i] == player && board[i+3] == player && board[i+6] == player) return true;
    }
    // Check diagonals
    if (board[0] == player && board[4] == player && board[8] == player) return true;
    if (board[2] == player && board[4] == player && board[6] == player) return true;
    return false;
}

bool TicTacToe::checkTie() {
    for (char cell : board) {
        if (cell == EMPTY) return false;
    }
    return true;
}

// ==================== DB Methods ====================
void TicTacToe::connectToDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tictactoe.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", "Could not open database: " + db.lastError().text());
    } else {
        qDebug() << "Database opened successfully.";
    }
}

void TicTacToe::createTablesIfNeeded() {
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "username TEXT UNIQUE NOT NULL,"
                    "password TEXT NOT NULL)")) {
        qDebug() << "Error creating users table:" << query.lastError().text();
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS match_history ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "timestamp TEXT NOT NULL,"
                    "username TEXT NOT NULL,"
                    "winner TEXT NOT NULL,"
                    "result TEXT NOT NULL)")) {
        qDebug() << "Error creating match_history table:" << query.lastError().text();
    }
}

void TicTacToe::saveMatchResult(const QString &winner, const QString &result) {
    QSqlQuery query;
    query.prepare("INSERT INTO match_history (timestamp, username, winner, result) "
                  "VALUES (:timestamp, :username, :winner, :result)");
    query.bindValue(":timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":username", loggedInUser); // Using loggedInUser
    query.bindValue(":winner", winner);
    query.bindValue(":result", result);

    if (!query.exec()) {
        qDebug() << "Error saving match result:" << query.lastError().text();
    }
}

void TicTacToe::loadMatchHistory() {
    matchHistoryTable->setRowCount(0); // Clear existing data

    QSqlQuery query;
    query.prepare("SELECT timestamp, username, winner, result FROM match_history WHERE username = :username ORDER BY timestamp DESC");
    query.bindValue(":username", loggedInUser); // Using loggedInUser

    if (!query.exec()) {
        qDebug() << "Error loading match history:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        matchHistoryTable->insertRow(row);
        for (int col = 0; col < 4; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            item->setTextAlignment(Qt::AlignCenter);
            matchHistoryTable->setItem(row, col, item);
        }
        ++row;
    }
}

void TicTacToe::registerAccount() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Registration Failed", "Username and password cannot be empty!");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        QMessageBox::warning(this, "Registration Failed", "Username already exists!");
        usernameEdit->clear();
        passwordEdit->clear();
    } else {
        QMessageBox::information(this, "Registration Success", "Account created successfully!");
        usernameEdit->clear();
        passwordEdit->clear();
    }
}

void TicTacToe::guestLogin() {
    loggedInUser = "Player 1";
    guestMode = true;
    stackedWidget->setCurrentIndex(1); // Mode selection
    usernameEdit->clear();
    passwordEdit->clear();
    // MODIFIED: Hide history button for guests - This logic is within setupUI based on guestMode
}

void TicTacToe::handleLogin() {
    QString username = usernameEdit->text(); // Assuming usernameEdit and passwordEdit are accessible here
    QString password = passwordEdit->text();

    QSqlQuery query;
    query.prepare("SELECT username FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (query.exec() && query.next()) {
        loggedInUser = username;
        QMessageBox::information(this, "Login Success", "Welcome, " + username + "!");
        stackedWidget->setCurrentIndex(1); // Mode selection screen
        usernameEdit->clear();
        passwordEdit->clear();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    }
}

void TicTacToe::setPlayerVsPlayer() {
    mode = 1;
    stackedWidget->setCurrentIndex(3); // Go to Player 2 name entry
}

void TicTacToe::setPlayerVsAI() {
    mode = 2;
    stackedWidget->setCurrentIndex(2); // Go to AI difficulty selection
}

void TicTacToe::setDifficultyEasy() {
    difficulty = 1;
    player2Name = "Easy AI";
    stackedWidget->setCurrentIndex(5); // Go to game board
    resetGame();
}

void TicTacToe::setDifficultyMedium() {
    difficulty = 2;
    player2Name = "Medium AI";
    stackedWidget->setCurrentIndex(5); // Go to game board
    resetGame();
}

void TicTacToe::setDifficultyHard() {
    difficulty = 3;
    player2Name = "Hard AI";
    stackedWidget->setCurrentIndex(5); // Go to game board
    resetGame();
}

void TicTacToe::startPvPWithNames() {
    player1Name = loggedInUser.isEmpty() ? "Player 1" : loggedInUser;
    player2Name = player2NameEdit->text();
    if (player2Name.isEmpty()) {
        player2Name = "Player 2";
    }
    stackedWidget->setCurrentIndex(5); // Go to game board
    resetGame();
}

void TicTacToe::applyGameSettings() {
    totalGames = totalGamesSpinBox->value();
    gamesToWin = gamesToWinSpinBox->value();

    if (gamesToWin > totalGames) {
        QMessageBox::warning(this, "Settings Error", "Games to win cannot be greater than total games!");
        gamesToWinSpinBox->setValue(totalGames);
        return;
    }
    QMessageBox::information(this, "Settings Applied", "Game settings updated successfully!");
}

void TicTacToe::logout() {
    loggedInUser.clear();
    guestMode = false;
    stackedWidget->setCurrentIndex(0); // Back to login screen
    resetGame(); // Reset game state on logout
}

void TicTacToe::backToModeSelection() {
    stackedWidget->setCurrentIndex(1); // Back to mode selection
}

void TicTacToe::handleButtonClick(int index) {
    makeMove(index);
}

void TicTacToe::toggleNightMode() {
    if (selectedTheme == "Dark") {
        selectedTheme = "Light";
    } else {
        selectedTheme = "Dark";
    }
    applyStyleSheet();
}

void TicTacToe::toggleScoreboard() {
    // This function will toggle visibility of scoreboard labels
    // The exact implementation depends on how scoreLabel and other score elements are managed
    // For simplicity, let's assume scoreLabel visibility is toggled.
    if (scoreLabel->isVisible()) {
        scoreLabel->hide();
    } else {
        scoreLabel->show();
    }
}

void TicTacToe::applyStyleSheet() {
    QString styleSheet;
    QString logoPath;

    if (selectedTheme == "Dark") {
        styleSheet = "QWidget { background-color: #333; color: #eee; } "
                     "QPushButton { background-color: #555; color: #eee; border: 1px solid #777; border-radius: 5px; padding: 10px; } "
                     "QPushButton:hover { background-color: #777; } "
                     "QLineEdit { background-color: #444; color: #eee; border: 1px solid #666; border-radius: 5px; padding: 5px; } "
                     "QLabel { color: #eee; } "
                     "QTableWidget { background-color: #444; color: #eee; gridline-color: #666; } "
                     "QHeaderView::section { background-color: #555; color: #eee; } "
                     "QSpinBox { background-color: #444; color: #eee; border: 1px solid #666; border-radius: 5px; padding: 5px; } "
                     "QComboBox { background-color: #444; color: #eee; border: 1px solid #666; border-radius: 5px; padding: 5px; } ";
        logoPath = ":/logos/dark_logo.png"; // Assuming dark_logo.png in resources
    } else if (selectedTheme == "Red") {
        styleSheet = "QWidget { background-color: #ffcccc; color: #333; } "
                     "QPushButton { background-color: #ff6666; color: white; border: 1px solid #cc0000; border-radius: 5px; padding: 10px; } "
                     "QPushButton:hover { background-color: #ff3333; } "
                     "QLineEdit { background-color: #ffe6e6; color: #333; border: 1px solid #ff9999; border-radius: 5px; padding: 5px; } "
                     "QLabel { color: #333; } "
                     "QTableWidget { background-color: #ffe6e6; color: #333; gridline-color: #ff9999; } "
                     "QHeaderView::section { background-color: #ff6666; color: white; } "
                     "QSpinBox { background-color: #ffe6e6; color: #333; border: 1px solid #ff9999; border-radius: 5px; padding: 5px; } "
                     "QComboBox { background-color: #ffe6e6; color: #333; border: 1px solid #ff9999; border-radius: 5px; padding: 5px; } ";
        logoPath = ":/logos/red_logo.png"; // Assuming red_logo.png in resources
    } else if (selectedTheme == "Blue") {
        styleSheet = "QWidget { background-color: #ccddff; color: #333; } "
                     "QPushButton { background-color: #6699ff; color: white; border: 1px solid #0033cc; border-radius: 5px; padding: 10px; } "
                     "QPushButton:hover { background-color: #3366ff; } "
                     "QLineEdit { background-color: #e6f0ff; color: #333; border: 1px solid #99ccff; border-radius: 5px; padding: 5px; } "
                     "QLabel { color: #333; } "
                     "QTableWidget { background-color: #e6f0ff; color: #333; gridline-color: #99ccff; } "
                     "QHeaderView::section { background-color: #6699ff; color: white; } "
                     "QSpinBox { background-color: #e6f0ff; color: #333; border: 1px solid #99ccff; border-radius: 5px; padding: 5px; } "
                     "QComboBox { background-color: #e6f0ff; color: #333; border: 1px solid #99ccff; border-radius: 5px; padding: 5px; } ";
        logoPath = ":/logos/blue_logo.png"; // Assuming blue_logo.png in resources
    } else if (selectedTheme == "Green") {
        styleSheet = "QWidget { background-color: #ccffcc; color: #333; } "
                     "QPushButton { background-color: #66cc66; color: white; border: 1px solid #009900; border-radius: 5px; padding: 10px; } "
                     "QPushButton:hover { background-color: #33cc33; } "
                     "QLineEdit { background-color: #e6ffe6; color: #333; border: 1px solid #99ff99; border-radius: 5px; padding: 5px; } "
                     "QLabel { color: #333; } "
                     "QTableWidget { background-color: #e6ffe6; color: #333; gridline-color: #99ff99; } "
                     "QHeaderView::section { background-color: #66cc66; color: white; } "
                     "QSpinBox { background-color: #e6ffe6; color: #333; border: 1px solid #99ff99; border-radius: 5px; padding: 5px; } "
                     "QComboBox { background-color: #e6ffe6; color: #333; border: 1px solid #99ff99; border-radius: 5px; padding: 5px; } ";
        logoPath = ":/logos/green_logo.png"; // Assuming green_logo.png in resources
    }
    else { // Light (Default)
        styleSheet = "QWidget { background-color: #f0f0f0; color: #333; } "
                     "QPushButton { background-color: #eee; color: #333; border: 1px solid #ccc; border-radius: 5px; padding: 10px; } "
                     "QPushButton:hover { background-color: #ddd; } "
                     "QLineEdit { background-color: white; color: #333; border: 1px solid #ccc; border-radius: 5px; padding: 5px; } "
                     "QLabel { color: #333; } "
                     "QTableWidget { background-color: white; color: #333; gridline-color: #ccc; } "
                     "QHeaderView::section { background-color: #eee; color: #333; } "
                     "QSpinBox { background-color: white; color: #333; border: 1px solid #ccc; border-radius: 5px; padding: 5px; } "
                     "QComboBox { background-color: white; color: #333; border: 1px solid #ccc; border-radius: 5px; padding: 5px; } ";
        logoPath = ":/logos/light_logo.png"; // Assuming light_logo.png in resources
    }
    qApp->setStyleSheet(styleSheet);

    // Update logo based on theme
    QLabel *logoLabel = findChild<QLabel*>("themeLogo");
    if (logoLabel) {
        QPixmap pixmap(logoPath);
        logoLabel->setPixmap(pixmap.scaled(logoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void TicTacToe::updateScoreboard() {
    scoreLabel->setText(QString("Score: %1 (X) %2 - %3 %4 (O) | Ties: %5")
                            .arg(player1Name).arg(player1Wins).arg(player2Wins).arg(player2Name).arg(ties));
}

void TicTacToe::handleSurrender() {
    QString winner;
    QString result = "Surrender";

    if (mode == 2) { // Player vs AI
        winner = (currentPlayer == PLAYER2) ? "AI" : player2Name; // AI surrenders or Player surrenders to AI
    } else { // Player vs Player
        winner = (currentPlayer == PLAYER1) ? player2Name : player1Name; // Player 2 wins if Player 1 surrenders, vice versa
    }

    saveMatchResult(winner, result);
    QMessageBox::information(this, "Surrender", QString("🏳️ %1 wins by surrender!").arg(winner));
    resetGame();
}