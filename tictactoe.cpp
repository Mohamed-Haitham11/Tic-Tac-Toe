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
// ==================== Constructor ====================
TicTacToe::TicTacToe(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    connectToDatabase();
    createTablesIfNeeded();
    applyStyleSheet();
    resetGame();
}
QString currentUsername ;

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

    QLabel *loginTitle = new QLabel("Tic Tac Toe Login", this);
    loginTitle->setAlignment(Qt::AlignCenter);
    loginTitle->setFont(QFont("Arial", 18, QFont::Bold));

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Username");
    usernameEdit->setMinimumHeight(40);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(40);

    QPushButton *loginButton = new QPushButton("Login", this);
    loginButton->setMinimumHeight(50);
    connect(loginButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::handleLogin);

    QPushButton *registerButton = new QPushButton("Register", this);
    registerButton->setMinimumHeight(40);
    connect(registerButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::registerAccount);

    QPushButton *guestButton = new QPushButton("Play as Guest", this);
    guestButton->setMinimumHeight(40);
    connect(guestButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::guestLogin);

    themeSelector = new QComboBox(this);
    themeSelector->addItems({"Light", "Dark", "Blue", "Plywood", "S.P.Q.R", "Carthago", "Frosted Glass", "Ancient Egypt", "Seljuk Empire","8-Bit","Cyber Enhanced"});
    themeSelector->setCurrentText("Light");
    connect(themeSelector, &QComboBox::currentTextChanged, this, [this](const QString &theme) {
        selectedTheme = theme;
        applyStyleSheet();
    });

    loginLayout->addSpacing(10);
    loginLayout->addWidget(loginTitle);
    loginLayout->addSpacing(20);
    loginLayout->addWidget(usernameEdit);
    loginLayout->addWidget(passwordEdit);
    loginLayout->addSpacing(20);
    loginLayout->addWidget(loginButton);
    loginLayout->addWidget(registerButton);
    loginLayout->addWidget(guestButton);
    loginLayout->addSpacing(10);
    loginLayout->addWidget(themeSelector);
    loginLayout->addStretch();
    stackedWidget->addWidget(loginWidget);
    // === Mode Selection Screen (Index 1) ===
    QWidget *modeSelectionWidget = new QWidget();
    QVBoxLayout *modeSelectionLayout = new QVBoxLayout(modeSelectionWidget);

    QLabel *modeSelectionTitle = new QLabel("Select Game Mode", this);
    modeSelectionTitle->setAlignment(Qt::AlignCenter);
    modeSelectionTitle->setFont(QFont("Arial", 24, QFont::Bold));

    QPushButton *pvpButton = new QPushButton("Player vs Player", this);
    QPushButton *pveButton = new QPushButton("Player vs AI", this);

    QString bigButtonStyle = "QPushButton { padding: 30px; font-size: 18px; font-weight: bold; }";
    pvpButton->setStyleSheet(bigButtonStyle);
    pveButton->setStyleSheet(bigButtonStyle);

    connect(pvpButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::setPlayerVsPlayer);
    connect(pveButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::setPlayerVsAI);
    QPushButton *historyButton = new QPushButton("View Match History", this);
    historyButton->setMinimumHeight(40);
    connect(historyButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::loadMatchHistory);
    QPushButton *logoutButtonMode = new QPushButton("Logout", this);
    logoutButtonMode->setMaximumWidth(100);
    connect(logoutButtonMode, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::logout);
    QPushButton *backToLoginButton = new QPushButton("Back", this);
    backToLoginButton->setMaximumWidth(100);
    connect(backToLoginButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, [this]() {
        stackedWidget->setCurrentIndex(0); // Go back to login screen
    });

    /*QPushButton *modeNightModeButton = new QPushButton("Night Mode", this);
    modeNightModeButton->setMaximumWidth(100);
    connect(modeNightModeButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::toggleNightMode);*/

    modeSelectionLayout->addWidget(modeSelectionTitle);
    modeSelectionLayout->addSpacing(30);
    modeSelectionLayout->addWidget(pvpButton);
    modeSelectionLayout->addWidget(pveButton);
    modeSelectionLayout->addSpacing(20);
    modeSelectionLayout->addWidget(historyButton);
    modeSelectionLayout->addStretch();
    QHBoxLayout *modeBtnsLayout = new QHBoxLayout();
    modeBtnsLayout->addWidget(logoutButtonMode);
    //modeBtnsLayout->addWidget(modeNightModeButton);
    modeSelectionLayout->addLayout(modeBtnsLayout);
    modeBtnsLayout->addWidget(backToLoginButton);
    // === Game Screen (Index 2) ===
    QWidget *gameWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(gameWidget);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backButton = new QPushButton("Back", this);
    backButton->setMaximumWidth(100);
    connect(backButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::backToModeSelection);

    scoreboardToggleButton = new QPushButton("Show Scoreboard", this);
    scoreboardToggleButton->setMaximumWidth(150);
    connect(scoreboardToggleButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::toggleScoreboard);

    QPushButton *logoutButton = new QPushButton("Logout", this);
    logoutButton->setMaximumWidth(100);
    connect(logoutButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::logout);

    /*nightModeButton = new QPushButton("Night Mode", this);
    nightModeButton->setMaximumWidth(100);
    connect(nightModeButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::toggleNightMode);*/

    headerLayout->addWidget(backButton);
    headerLayout->addWidget(scoreboardToggleButton);
    headerLayout->addStretch();
    //headerLayout->addWidget(nightModeButton);
    headerLayout->addWidget(logoutButton);
    mainLayout->addLayout(headerLayout);

    scoreLabel = new QLabel(this);
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setFont(QFont("Arial", 14, QFont::Bold));
    scoreLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; padding: 10px; }");
    scoreLabel->setVisible(scoreboardVisible);
    mainLayout->addWidget(scoreLabel);

    QGridLayout *gridLayout = new QGridLayout();
    buttonGroup = new QButtonGroup(this);

    for (int i = 0; i < 9; i++) {
        QPushButton *button = new QPushButton("", this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setMinimumSize(100, 100);
        button->setFont(QFont("Arial", 36, QFont::Bold));
        button->setStyleSheet("QPushButton { border: 2px solid #aaa; }");
        gridLayout->addWidget(button, i / 3, i % 3);
        buttonGroup->addButton(button, i);
    }

    connect(buttonGroup, &QButtonGroup::buttonClicked, this, [this](QAbstractButton *button) {
        handleButtonClick(buttonGroup->id(button));
    });

    mainLayout->addLayout(gridLayout);

    statusLabel = new QLabel("Player O's turn", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFont(QFont("Arial", 16));
    mainLayout->addWidget(statusLabel);

    // === Difficulty Selection (Index 3) ===
    QWidget *difficultyWidget = new QWidget();
    QVBoxLayout *difficultyLayout = new QVBoxLayout(difficultyWidget);

    QLabel *difficultyTitle = new QLabel("Select AI Difficulty", this);
    difficultyTitle->setAlignment(Qt::AlignCenter);
    difficultyTitle->setFont(QFont("Arial", 24, QFont::Bold));

    QPushButton *easyButton = new QPushButton("Easy", this);
    QPushButton *mediumButton = new QPushButton("Medium", this);
    QPushButton *hardButton = new QPushButton("Hard", this);

    easyButton->setStyleSheet(bigButtonStyle);
    mediumButton->setStyleSheet(bigButtonStyle);
    hardButton->setStyleSheet(bigButtonStyle);
    QPushButton *backToModeButton = new QPushButton("Back", this);
    backToModeButton->setMaximumWidth(100);
    connect(backToModeButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, [this]() {
        stackedWidget->setCurrentIndex(1); // Back to mode selection
    });
    connect(easyButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::setDifficultyEasy);
    connect(mediumButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::setDifficultyMedium);
    connect(hardButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::setDifficultyHard);

    difficultyLayout->addWidget(difficultyTitle);
    difficultyLayout->addSpacing(30);
    difficultyLayout->addWidget(easyButton);
    difficultyLayout->addWidget(mediumButton);
    difficultyLayout->addWidget(hardButton);
    difficultyLayout->addSpacing(20);
    difficultyLayout->addWidget(backToModeButton);
    /*QPushButton *diffNightModeButton = new QPushButton("Night Mode", this);
    diffNightModeButton->setMaximumWidth(100);
    connect(diffNightModeButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::toggleNightMode);

    difficultyLayout->addWidget(diffNightModeButton);*/

    // === Player 2 Name Input (Index 4) ===
    QWidget *nameInputWidget = new QWidget();
    QVBoxLayout *nameInputLayout = new QVBoxLayout(nameInputWidget);

    QLabel *nameInputTitle = new QLabel("Enter Player 2 Name", this);
    nameInputTitle->setAlignment(Qt::AlignCenter);
    nameInputTitle->setFont(QFont("Arial", 24, QFont::Bold));

    player2NameEdit = new QLineEdit(this);
    player2NameEdit->setPlaceholderText("Player 2 Name");
    player2NameEdit->setMinimumHeight(40);

    QPushButton *startGameButton = new QPushButton("Start Game", this);
    startGameButton->setMinimumHeight(50);
    connect(startGameButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::startPvPWithNames);

    nameInputLayout->addWidget(nameInputTitle);
    nameInputLayout->addSpacing(30);
    nameInputLayout->addWidget(player2NameEdit);
    nameInputLayout->addWidget(startGameButton);
    QPushButton *backToSettingsButton = new QPushButton("Back", this);
    backToSettingsButton->setMaximumWidth(100);
    connect(backToSettingsButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, [this]() {
        stackedWidget->setCurrentIndex(5); // Go back to settings screen
    });
    nameInputLayout->addWidget(backToSettingsButton);

    // === Settings Screen (Index 5) ===
    QWidget *settingsWidget = new QWidget();
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsWidget);

    QLabel *settingsTitle = new QLabel("Game Settings", this);
    settingsTitle->setAlignment(Qt::AlignCenter);
    settingsTitle->setFont(QFont("Arial", 24, QFont::Bold));

    QHBoxLayout *totalGamesLayout = new QHBoxLayout();
    QLabel *totalGamesLabel = new QLabel("Total Games:", this);
    totalGamesSpinBox = new QSpinBox(this);
    totalGamesSpinBox->setRange(1, 10);
    totalGamesSpinBox->setValue(totalGames);
    totalGamesLayout->addWidget(totalGamesLabel);
    totalGamesLayout->addWidget(totalGamesSpinBox);

    QHBoxLayout *gamesToWinLayout = new QHBoxLayout();
    QLabel *gamesToWinLabel = new QLabel("Games to Win:", this);
    gamesToWinSpinBox = new QSpinBox(this);
    gamesToWinSpinBox->setRange(1, 10);
    gamesToWinSpinBox->setValue(gamesToWin);
    gamesToWinLayout->addWidget(gamesToWinLabel);
    gamesToWinLayout->addWidget(gamesToWinSpinBox);

    QPushButton *applySettingsButton = new QPushButton("Apply Settings", this);
    applySettingsButton->setMinimumHeight(50);
    connect(applySettingsButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, &TicTacToe::applyGameSettings);

    settingsLayout->addWidget(settingsTitle);
    settingsLayout->addSpacing(20);
    settingsLayout->addLayout(totalGamesLayout);
    settingsLayout->addLayout(gamesToWinLayout);
    settingsLayout->addWidget(applySettingsButton);
    QPushButton *backToModeButton2 = new QPushButton("Back", this);
    backToModeButton2->setMaximumWidth(100);
    connect(backToModeButton2, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, [this]() {
        stackedWidget->setCurrentIndex(1); // Back to mode selection
    });
    settingsLayout->addWidget(backToModeButton2);
    // === Match History Screen (Index 6) ===
    QWidget *historyWidget = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyWidget);

    QLabel *historyTitle = new QLabel("Match History", this);
    historyTitle->setAlignment(Qt::AlignCenter);
    historyTitle->setFont(QFont("Arial", 24, QFont::Bold));

    matchHistoryTable = new QTableWidget(this);
    matchHistoryTable->setColumnCount(5);
    matchHistoryTable->setHorizontalHeaderLabels({"Date", "Player 1", "Player 2", "Winner", "Result"});
    matchHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    matchHistoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(backButton, static_cast<void (QPushButton::*)()>(&QPushButton::clicked), this, [this]() {
        stackedWidget->setCurrentIndex(1); // Return to mode selection
    });

    historyLayout->addWidget(historyTitle);
    historyLayout->addWidget(matchHistoryTable);
    historyLayout->addWidget(backButton);

    // === Add Widgets to Stacked Widget ===
    stackedWidget->addWidget(loginWidget);          // 0
    stackedWidget->addWidget(modeSelectionWidget);  // 1
    stackedWidget->addWidget(gameWidget);           // 2
    stackedWidget->addWidget(difficultyWidget);     // 3
    stackedWidget->addWidget(nameInputWidget);      // 4
    stackedWidget->addWidget(settingsWidget);       // 5
    stackedWidget->addWidget(historyWidget);        // 6


    setCentralWidget(stackedWidget);
    setWindowTitle("Tic Tac Toe");
}


// ==================== Theme Application ====================
// (applyStyleSheet())
void TicTacToe::applyStyleSheet() {
    QString style;

    if (selectedTheme == "Dark") {
        style = "QWidget { background-color: #2D2D2D; color: #E0E0E0; }"
                "QLabel { color: #E0E0E0; }"
                "QLineEdit { background-color: #3D3D3D; color: #E0E0E0; border: 1px solid #5D5D5D; border-radius: 4px; padding: 5px; }"
                "QPushButton { background-color: #3D3D3D; color: #E0E0E0; border: 1px solid #5D5D5D; border-radius: 4px; padding: 8px; }"
                "QPushButton:hover { background-color: #4D4D4D; }"
                "QPushButton:pressed { background-color: #5D5D5D; }"
                "QSpinBox { background-color: #3D3D3D; color: #E0E0E0; border: 1px solid #5D5D5D; border-radius: 4px; padding: 5px; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #3D3D3D; color: #E0E0E0; border: 1px solid #555555; padding: 10px; }");
    }
    else if (selectedTheme == "Blue") {
        style = "QWidget { background-color: #1E1E40; color: #FFFFFF; }"
                "QLabel { color: #FFFFFF; }"
                "QLineEdit { background-color: #304070; color: #FFFFFF; border: 1px solid #506090; border-radius: 4px; padding: 5px; }"
                "QPushButton { background-color: #304070; color: #FFFFFF; border: 1px solid #506090; border-radius: 4px; padding: 8px; }"
                "QPushButton:hover { background-color: #4060A0; }"
                "QPushButton:pressed { background-color: #5070C0; }"
                "QSpinBox { background-color: #304070; color: #FFFFFF; border: 1px solid #506090; border-radius: 4px; padding: 5px; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #2B3A67; color: #FFFFFF; border: 1px solid #4060A0; padding: 10px; }");
    }
    else if (selectedTheme == "Plywood") {
        style = "QWidget { background-color: #D7C4A3; color: #4E342E; }"
                "QLabel { color: #4E342E; }"
                "QLineEdit, QSpinBox { background-color: #EFEBE9; color: #4E342E; border: 1px solid #8D6E63; border-radius: 4px; padding: 5px; }"
                "QPushButton { background-color: #BCAAA4; color: #3E2723; border: 1px solid #8D6E63; border-radius: 4px; padding: 8px; }"
                "QPushButton:hover { background-color: #A1887F; }"
                "QPushButton:pressed { background-color: #8D6E63; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #EFEBE9; color: #4E342E; border: 1px solid #8D6E63; padding: 10px; }");
    }
    else if (selectedTheme == "S.P.Q.R") {
        style = "QWidget { background-color: #6E1414; color: #FFD700; }"
                "QLabel { color: #FFD700; font-weight: bold; }"
                "QLineEdit, QSpinBox { background-color: #9E1B1B; color: #FFD700; border: 1px solid #B22222; border-radius: 4px; padding: 5px; }"
                "QPushButton { background-color: #800000; color: #FFD700; border: 1px solid #B22222; border-radius: 4px; padding: 8px; }"
                "QPushButton:hover { background-color: #A52A2A; }"
                "QPushButton:pressed { background-color: #B22222; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #9E1B1B; color: #FFD700; border: 1px solid #B22222; padding: 10px; }");
    }
    else if (selectedTheme == "Carthago") {
        style = "QWidget { background-color: #3E1F47; color: #FFFFFF; }"
                "QLabel { color: #FFFFFF; font-weight: bold; }"
                "QLineEdit, QSpinBox { background-color: #5C2E7E; color: #FFFFFF; border: 1px solid #8A4FAD; border-radius: 4px; padding: 5px; }"
                "QPushButton { background-color: #6A1B9A; color: #FFFFFF; border: 1px solid #9C4DCC; border-radius: 4px; padding: 8px; }"
                "QPushButton:hover { background-color: #7E57C2; }"
                "QPushButton:pressed { background-color: #9C4DCC; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #5C2E7E; color: #FFFFFF; border: 1px solid #9C4DCC; padding: 10px; }");
    }
    else if (selectedTheme == "Frosted Glass") {
        style = "QWidget { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #E0F7FA, stop:1 #B2EBF2); color: #003E57; }"
                "QLabel { color: #003E57; font-weight: bold; }"
                "QLineEdit, QSpinBox { background-color: rgba(255, 255, 255, 0.7); color: #003E57; border: 1px solid #B0BEC5; border-radius: 6px; padding: 5px; }"
                "QPushButton { background-color: rgba(255, 255, 255, 0.8); color: #004D60; border: 1px solid #90A4AE; border-radius: 6px; padding: 8px; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.9); }"
                "QPushButton:pressed { background-color: #CFD8DC; }";
        scoreLabel->setStyleSheet("QLabel { background-color: rgba(255, 255, 255, 0.6); color: #004D60; border: 1px solid #B0BEC5; padding: 10px; }");
    }
    else if (selectedTheme == "Ancient Egypt") {
        style = "QWidget { background-color: #F5E5B8; color: #5C432E; }"
                "QLabel { color: #5C432E; font-weight: bold; }"
                "QLineEdit, QSpinBox { background-color: #F9F3D2; color: #5C432E; border: 1px solid #CBB67C; border-radius: 5px; padding: 5px; }"
                "QPushButton { background-color: #E5C56E; color: #4E342E; border: 1px solid #B79850; border-radius: 5px; padding: 8px; }"
                "QPushButton:hover { background-color: #DFC276; }"
                "QPushButton:pressed { background-color: #CBB67C; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #F9F3D2; color: #5C432E; border: 1px solid #CBB67C; padding: 10px; }");
    }
    else if (selectedTheme == "Seljuk Empire") {
        style = "QWidget { background-color: #0D3B66; color: #FAF0E6; }"
                "QLabel { color: #FAF0E6; font-weight: bold; }"
                "QLineEdit, QSpinBox { background-color: #145DA0; color: #FFFFFF; border: 1px solid #1E81B0; border-radius: 6px; padding: 5px; }"
                "QPushButton { background-color: #1E81B0; color: #FFFFFF; border: 1px solid #63A4FF; border-radius: 6px; padding: 8px; }"
                "QPushButton:hover { background-color: #63A4FF; }"
                "QPushButton:pressed { background-color: #145DA0; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #145DA0; color: #FFFFFF; border: 1px solid #63A4FF; padding: 10px; }");
    }
    else if (selectedTheme == "Cyber Enhanced") {
        style = "QWidget { background-color: #0F0F0F; color: #00FFEA; }"
                "QLabel { color: #00FFEA; font-weight: bold; }"
                "QLineEdit, QSpinBox { background-color: #1F1F1F; color: #00FFEA; border: 1px solid #00FFEA; border-radius: 6px; padding: 6px; }"
                "QPushButton { background-color: #1F1F1F; color: #00FFEA; border: 1px solid #00FFEA; border-radius: 6px; padding: 8px; }"
                "QPushButton:hover { background-color: #00FFEA; color: #0F0F0F; }"
                "QPushButton:pressed { background-color: #0F0F0F; color: #00FFEA; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #1F1F1F; color: #00FFEA; border: 1px solid #00FFEA; padding: 10px; }");
    }
    else if (selectedTheme == "8-Bit") {
        style = "QWidget { background-color: #282828; color: #FFD700; font-family: 'Courier New'; }"
                "QLabel { color: #FFD700; font-weight: bold; }"
                "QLineEdit, QSpinBox { background-color: #404040; color: #FFD700; border: 2px solid #FFD700; border-radius: 0px; padding: 6px; font-family: 'Courier New'; }"
                "QPushButton { background-color: #404040; color: #FFD700; border: 2px solid #FFD700; padding: 8px; font-family: 'Courier New'; }"
                "QPushButton:hover { background-color: #606060; }"
                "QPushButton:pressed { background-color: #303030; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #303030; color: #FFD700; border: 2px solid #FFD700; padding: 10px; font-family: 'Courier New'; }");
    }

    else {
        // Light theme (default)
        style = "QWidget { background-color: #F5F5F5; color: #333333; }"
                "QLabel { color: #333333; }"
                "QLineEdit { background-color: #FFFFFF; color: #333333; border: 1px solid #CCCCCC; border-radius: 4px; padding: 5px; }"
                "QPushButton { background-color: #FFFFFF; color: #333333; border: 1px solid #CCCCCC; border-radius: 4px; padding: 8px; }"
                "QPushButton:hover { background-color: #EEEEEE; }"
                "QPushButton:pressed { background-color: #DDDDDD; }"
                "QSpinBox { background-color: #FFFFFF; color: #333333; border: 1px solid #CCCCCC; border-radius: 4px; padding: 5px; }";
        scoreLabel->setStyleSheet("QLabel { background-color: #F0F0F0; color: #333333; border: 1px solid #CCCCCC; padding: 10px; }");
    }

    setStyleSheet(style);

    for (int i = 0; i < 9; ++i) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
        if (button) {
            button->setStyleSheet("QPushButton { font-weight: bold; font-size: 24px; padding: 10px; }");
        }
    }
    QLabel *logo = findChild<QLabel*>("themeLogo");
    if (logo) {
        QString themeKey = selectedTheme.toLower().replace(" ", "_").replace(".", "");
        QString logoPath = QString(":/logos/%1.png").arg(themeKey);
        QPixmap pixmap(logoPath);
        logo->setPixmap(pixmap.scaled(logo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
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

// ==================== Login / Logout ====================
// (handleLogin(), logout(), backToModeSelection())
void TicTacToe::handleLogin() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();
    currentUsername = usernameEdit->text() ;
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Username and password cannot be empty!");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    if (query.exec() && query.next()) {
        loggedInUser = username;
        guestMode = false;
        stackedWidget->setCurrentIndex(1); // Go to mode selection

        // MODIFIED: Show history button for logged-in users
        QPushButton *historyButton = findChild<QPushButton *>("View Match History");
        if (historyButton) historyButton->show();

        usernameEdit->clear(); // MODIFIED: Clear after use
        passwordEdit->clear();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password!");
    }
}

void TicTacToe::logout() {
    loggedInUser.clear();
    guestMode = false;
    usernameEdit->clear();
    passwordEdit->clear();
    stackedWidget->setCurrentIndex(0); // Back to login screen
}


void TicTacToe::backToModeSelection() {
    player1Wins = 0;
    player2Wins = 0;
    ties = 0;
    updateScoreboard();
    stackedWidget->setCurrentIndex(1); // Go back to mode selection screen
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
        // MODIFIED: Guest mode skips name entry and uses default names
        if (guestMode) {
            player1Name = "Player 1";
            player2Name = "Player 2";
            stackedWidget->setCurrentIndex(2); // Go directly to game screen
            resetGame();
        } else {
            stackedWidget->setCurrentIndex(4); // Go to player name input screen
        }
    } else {
        stackedWidget->setCurrentIndex(2); // Go to game screen
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

    std::random_device rd;
    std::mt19937 gen(rd());
    if (gen() % 2 == 0) {
        currentPlayer = PLAYER1;
        if (mode == 2) {
            QTimer::singleShot(500, this, [this]() { makeAIMove(); });
        }
    } else {
        currentPlayer = PLAYER2;
    }

    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
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
    updateBoard();

    if (checkWin(currentPlayer)) {
        if (currentPlayer == PLAYER1) player1Wins++;
        else player2Wins++;
        updateScoreboard();

        // Check if either player has won the series
        if (player1Wins >= gamesToWin || player2Wins >= gamesToWin) {
            QString winner = player1Wins >= gamesToWin ? player1Name : player2Name;
            gameOver(QString("%1 wins the series %2-%3!")
                         .arg(winner)
                         .arg(qMax(player1Wins, player2Wins))
                         .arg(qMin(player1Wins, player2Wins)), true);
            return;
        }

        // Check if maximum games reached
        if ((player1Wins + player2Wins + ties) >= totalGames) {
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
            return;
        }

        gameOver(QString("%1 (%2) wins!").arg(currentPlayer == PLAYER1 ? player1Name : player2Name).arg(currentPlayer));
        return;
    }

    if (checkTie()) {
        ties++;
        updateScoreboard();

        // Check if maximum games reached
        if ((player1Wins + player2Wins + ties) >= totalGames) {
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
    std::vector<int> emptySlots;
    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            emptySlots.push_back(i);
        }
    }

    if (!emptySlots.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, emptySlots.size() - 1);
        int index = emptySlots[dis(gen)];
        makeMove(index);
    }
}

void TicTacToe::mediumMove() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);

    if (dis(gen) < 50) {
        hardMove();
    } else {
        easyMove();
    }
}

void TicTacToe::hardMove() {
    int bestScore = std::numeric_limits<int>::min();
    int bestMove = -1;

    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            board[i] = PLAYER1;
            int score = minimax(0, false);
            board[i] = EMPTY;

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


// ==================== Win / Tie Check ====================
// (checkWin(), checkTie(), gameOver())
void TicTacToe::gameOver(const QString &message, bool seriesOver) {
    statusLabel->setText(message);

    if (!scoreboardVisible)
        toggleScoreboard();

    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
        if (button) button->setEnabled(false);
    }

    // Save only when the full match is over
    if (!guestMode && seriesOver) {
        QString winner;
        QString loser;
        QString result;

        if (player1Wins > player2Wins) {
            winner = player1Name;
            loser = player2Name;
            result = "Win";
        } else if (player2Wins > player1Wins) {
            winner = player2Name;
            loser = player1Name;
            result = "Win";
        } else {
            winner = "-";
            loser = "-";
            result = "Tie";
        }

        if (mode == 1) {
            saveMatchResult(winner, result);
        } else if (mode == 2) {
            if (result == "Tie") {
                saveMatchResult("Player", "Tie");
            } else if (winner != "AI") {
                saveMatchResult(winner, "Win");
            } else {
                saveMatchResult("Player", "Defeat");
            }
        }
    }

    if (seriesOver)
        QTimer::singleShot(3000, this, &TicTacToe::backToModeSelection);
    else
        QTimer::singleShot(2000, this, &TicTacToe::resetGame);
}



bool TicTacToe::checkWin(char player) {
    for (int i = 0; i < 3; i++) {
        if (board[i*3] == player && board[i*3+1] == player && board[i*3+2] == player) {
            return true;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (board[i] == player && board[i+3] == player && board[i+6] == player) {
            return true;
        }
    }

    if (board[0] == player && board[4] == player && board[8] == player) {
        return true;
    }
    if (board[2] == player && board[4] == player && board[6] == player) {
        return true;
    }

    return false;
}

bool TicTacToe::checkTie() {
    for (int i = 0; i < 9; i++) {
        if (board[i] == EMPTY) {
            return false;
        }
    }
    return true;
}
// ==================== Board / UI Updates ====================
// (updateBoard(), updateStatus(), toggleNightMode(), toggleScoreboard())
void TicTacToe::updateBoard() {
    for (int i = 0; i < 9; i++) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
        if (button) {
            button->setText(board[i] == EMPTY ? "" : QString(board[i]));
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
void TicTacToe::toggleNightMode() {
    nightMode = !nightMode;
    applyStyleSheet();

    if (nightModeButton) {
        nightModeButton->setText(nightMode ? "Day Mode" : "Night Mode");
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
// (handleButtonClick())
void TicTacToe::handleButtonClick(int index) {
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
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "username TEXT UNIQUE NOT NULL,"
               "password TEXT NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS matches ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "player1 TEXT,"
               "player2 TEXT,"
               "winner TEXT,"
               "result TEXT,"
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");
}


void TicTacToe::saveMatchResult(const QString &winner, const QString &result) {
    if (!db.isOpen()) return;

    QSqlQuery query;
    query.prepare("INSERT INTO matches (player1, player2, winner, result) "
                  "VALUES (:player1, :player2, :winner, :result)");
    query.bindValue(":player1", player1Name);
    query.bindValue(":player2", player2Name);
    query.bindValue(":winner", winner);
    query.bindValue(":result", result);
    query.exec();
}
void TicTacToe::loadMatchHistory() {
    matchHistoryTable->clearContents();
    matchHistoryTable->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT player1, player2, winner, result, timestamp "
                  "FROM matches WHERE player1 = :user OR player2 = :user "
                  "ORDER BY timestamp DESC");
    query.bindValue(":user", currentUsername);
    query.exec();

    int row = 0;
    while (query.next()) {
        matchHistoryTable->insertRow(row);

        for (int col = 0; col < 5; ++col) {
            QString text = query.value(col == 0 ? 4 : col - 1).toString(); // timestamp first
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            matchHistoryTable->setItem(row, col, item);
        }
        ++row;
    }

    stackedWidget->setCurrentIndex(6);  // Switch to match history screen
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
    usernameEdit->clear(); // MODIFIED: Clear after use
    passwordEdit->clear();

    // MODIFIED: Hide history button for guests
}
/*
REAL full function bodies will be attached part-by-part.
This is the final full layout for perfect building.
*/