#include "tictactoe.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QAbstractItemView>

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
    connect(loginButton, &QPushButton::clicked, this, &TicTacToe::handleLogin);
    QPushButton *registerButton = new QPushButton("Register", this);
    registerButton->setMinimumHeight(40);
    connect(registerButton, &QPushButton::clicked, this, &TicTacToe::registerAccount);

    QPushButton *guestButton = new QPushButton("Play as Guest", this);
    guestButton->setMinimumHeight(40);
    connect(guestButton, &QPushButton::clicked, this, &TicTacToe::guestLogin);

    themeSelector = new QComboBox(this);
    themeSelector->addItems({"Light", "Dark", "Blue", "Plywood", "S.P.Q.R", "Carthago", "Frosted Glass", "Ancient Egypt", "Seljuk Empire","8-Bit","Cyber Enhanced"});
    themeSelector->setCurrentText("Light");
    connect(themeSelector, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, [this](const QString &theme) {
                qDebug() << "Theme changed to:" << theme;  // Debug output
                selectedTheme = theme;
                qDebug() << "selectedTheme is now:" << selectedTheme;  // Debug output
                applyStyleSheet();
                qDebug() << "applyStyleSheet() called";  // Debug output
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
    connect(pvpButton, &QPushButton::clicked, this, &TicTacToe::setPlayerVsPlayer);
    connect(pveButton, &QPushButton::clicked, this, &TicTacToe::setPlayerVsAI);

    QPushButton *historyButton = new QPushButton("View Match History", this);
    historyButton->setObjectName("View Match History");
    historyButton->setMinimumHeight(40);
    connect(historyButton, &QPushButton::clicked, this, &TicTacToe::loadMatchHistory);

    QPushButton *logoutButtonMode = new QPushButton("Logout", this);
    logoutButtonMode->setMaximumWidth(100);
    connect(logoutButtonMode, &QPushButton::clicked, this, &TicTacToe::logout);

    QPushButton *deleteAccountButton = new QPushButton("Delete Account", this);
    deleteAccountButton->setObjectName("DeleteAccountButton");
    deleteAccountButton->setMaximumWidth(140);
    deleteAccountButton->setMinimumHeight(40);
    deleteAccountButton->hide();
    connect(deleteAccountButton, &QPushButton::clicked, this, &TicTacToe::deleteAccount);

    modeSelectionLayout->addWidget(modeSelectionTitle);
    modeSelectionLayout->addSpacing(30);
    modeSelectionLayout->addWidget(pvpButton);
    modeSelectionLayout->addWidget(pveButton);
    modeSelectionLayout->addSpacing(20);
    modeSelectionLayout->addWidget(historyButton);
    modeSelectionLayout->addStretch();

    QHBoxLayout *modeBtnsLayout = new QHBoxLayout();
    modeBtnsLayout->addWidget(logoutButtonMode);
    modeBtnsLayout->addWidget(deleteAccountButton);
    modeSelectionLayout->addLayout(modeBtnsLayout);

    stackedWidget->addWidget(modeSelectionWidget);

    // === Game Screen (Index 2) ===
    QWidget *gameWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(gameWidget);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backButton = new QPushButton("Back", this);
    backButton->setMaximumWidth(100);
    connect(backButton, &QPushButton::clicked, this, &TicTacToe::backToModeSelection);

    scoreboardToggleButton = new QPushButton("Show Scoreboard", this);
    scoreboardToggleButton->setMaximumWidth(150);
    connect(scoreboardToggleButton, &QPushButton::clicked, this, &TicTacToe::toggleScoreboard);

    // Video Recording Controls
    recordButton = new QPushButton("Start Recording", this);
    recordButton->setMaximumWidth(130);
    recordButton->setStyleSheet("QPushButton { background-color: #ff4444; color: white; font-weight: bold; }");
    connect(recordButton, &QPushButton::clicked, this, [this]() {
        if (isRecording) {
            stopVideoRecording();
        } else {
            startVideoRecording();
        }
    });

    recordingStatusLabel = new QLabel("⚫ Not Recording", this);
    recordingStatusLabel->setStyleSheet("color: gray;");

    QPushButton *surrenderButton = new QPushButton("🏳️ Surrender", this);
    surrenderButton->setObjectName("SurrenderButton");
    surrenderButton->setMaximumWidth(130);
    connect(surrenderButton, &QPushButton::clicked, this, &TicTacToe::handleSurrender);

    QPushButton *logoutButton = new QPushButton("Logout", this);
    logoutButton->setMaximumWidth(100);
    connect(logoutButton, &QPushButton::clicked, this, &TicTacToe::logout);

    headerLayout->addWidget(backButton);
    headerLayout->addWidget(scoreboardToggleButton);
    headerLayout->addWidget(recordButton);
    headerLayout->addWidget(recordingStatusLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(surrenderButton);
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
    connect(buttonGroup, &QButtonGroup::buttonClicked,
            this, [this](QAbstractButton* button) {
                int id = buttonGroup->id(button);
                handleButtonClick(id);
            });
    mainLayout->addLayout(gridLayout);

    statusLabel = new QLabel("Player O's turn", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFont(QFont("Arial", 16));
    mainLayout->addWidget(statusLabel);

    // Video playback controls (initially hidden)
    QHBoxLayout *videoControlsLayout = new QHBoxLayout();
    playVideoButton = new QPushButton("Play Video", this);
    playVideoButton->setMaximumWidth(120);
    playVideoButton->hide();
    connect(playVideoButton, &QPushButton::clicked, this, &TicTacToe::playRecordedVideo);

    stopVideoButton = new QPushButton("Stop Video", this);
    stopVideoButton->setMaximumWidth(120);
    stopVideoButton->hide();
    connect(stopVideoButton, &QPushButton::clicked, this, &TicTacToe::stopVideoPlayback);

    videoStatusLabel = new QLabel("", this);
    videoStatusLabel->setAlignment(Qt::AlignCenter);

    videoProgressSlider = new QSlider(Qt::Horizontal, this);
    videoProgressSlider->setRange(0, 100);
    videoProgressSlider->hide();

    videoControlsLayout->addWidget(playVideoButton);
    videoControlsLayout->addWidget(stopVideoButton);
    videoControlsLayout->addWidget(videoProgressSlider);
    videoControlsLayout->addStretch();
    videoControlsLayout->addWidget(videoStatusLabel);
    mainLayout->addLayout(videoControlsLayout);

    stackedWidget->addWidget(gameWidget);

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
    connect(backToModeButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });

    connect(easyButton, &QPushButton::clicked, this, &TicTacToe::setDifficultyEasy);
    connect(mediumButton, &QPushButton::clicked, this, &TicTacToe::setDifficultyMedium);
    connect(hardButton, &QPushButton::clicked, this, &TicTacToe::setDifficultyHard);

    difficultyLayout->addWidget(difficultyTitle);
    difficultyLayout->addSpacing(30);
    difficultyLayout->addWidget(easyButton);
    difficultyLayout->addWidget(mediumButton);
    difficultyLayout->addWidget(hardButton);
    difficultyLayout->addSpacing(20);
    difficultyLayout->addWidget(backToModeButton);

    stackedWidget->addWidget(difficultyWidget);

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
    connect(startGameButton, &QPushButton::clicked, this, &TicTacToe::startPvPWithNames);

    nameInputLayout->addWidget(nameInputTitle);
    nameInputLayout->addSpacing(30);
    nameInputLayout->addWidget(player2NameEdit);
    nameInputLayout->addWidget(startGameButton);

    QPushButton *backToSettingsButton = new QPushButton("Back", this);
    backToSettingsButton->setMaximumWidth(100);
    connect(backToSettingsButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(5);
    });
    nameInputLayout->addWidget(backToSettingsButton);

    stackedWidget->addWidget(nameInputWidget);

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
    totalGamesSpinBox->setValue(3);
    totalGamesLayout->addWidget(totalGamesLabel);
    totalGamesLayout->addWidget(totalGamesSpinBox);

    QHBoxLayout *gamesToWinLayout = new QHBoxLayout();
    QLabel *gamesToWinLabel = new QLabel("Games to Win:", this);
    gamesToWinSpinBox = new QSpinBox(this);
    gamesToWinSpinBox->setRange(1, 10);
    gamesToWinSpinBox->setValue(2);
    gamesToWinLayout->addWidget(gamesToWinLabel);
    gamesToWinLayout->addWidget(gamesToWinSpinBox);

    QPushButton *applySettingsButton = new QPushButton("Apply Settings", this);
    applySettingsButton->setMinimumHeight(50);
    connect(applySettingsButton, &QPushButton::clicked, this, &TicTacToe::applyGameSettings);

    settingsLayout->addWidget(settingsTitle);
    settingsLayout->addSpacing(20);
    settingsLayout->addLayout(totalGamesLayout);
    settingsLayout->addLayout(gamesToWinLayout);
    settingsLayout->addWidget(applySettingsButton);

    QPushButton *backToModeButton2 = new QPushButton("Back", this);
    backToModeButton2->setMaximumWidth(100);
    connect(backToModeButton2, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });
    settingsLayout->addWidget(backToModeButton2);

    stackedWidget->addWidget(settingsWidget);

    // === Match History Screen (Index 6) ===
    QWidget *historyWidget = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyWidget);

    QLabel *historyTitle = new QLabel("Match History", this);
    historyTitle->setAlignment(Qt::AlignCenter);
    historyTitle->setFont(QFont("Arial", 24, QFont::Bold));

    matchHistoryTable = new QTableWidget(this);
    matchHistoryTable->setColumnCount(6);
    matchHistoryTable->setHorizontalHeaderLabels({"Date", "Player 1", "Player 2", "Winner", "Result", "Video"});
    matchHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    matchHistoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QPushButton *openRecordedMatchesButton = new QPushButton("Replay Recorded Matches", this);
    openRecordedMatchesButton->setMinimumHeight(40);
    connect(openRecordedMatchesButton, &QPushButton::clicked, this, &TicTacToe::loadRecordedMatchesScreen);

    QPushButton *backFromHistoryButton = new QPushButton("Back", this);
    backFromHistoryButton->setMaximumWidth(100);
    connect(backFromHistoryButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });

    historyLayout->addWidget(historyTitle);
    historyLayout->addWidget(matchHistoryTable);
    historyLayout->addWidget(openRecordedMatchesButton);
    historyLayout->addWidget(backFromHistoryButton);

    stackedWidget->addWidget(historyWidget);

    // === Recorded Matches Screen (Index 7) ===
    QWidget *recordedMatchesWidget = new QWidget();
    QVBoxLayout *recordedLayout = new QVBoxLayout(recordedMatchesWidget);

    QLabel *recordedTitle = new QLabel("Recorded Match Playback", this);
    recordedTitle->setAlignment(Qt::AlignCenter);
    recordedTitle->setFont(QFont("Arial", 24, QFont::Bold));

    recordedMatchesTable = new QTableWidget(this);
    recordedMatchesTable->setColumnCount(7);
    recordedMatchesTable->setHorizontalHeaderLabels({"Match ID", "Player 1", "Player 2", "Winner", "Result", "Date", "Video"});
    recordedMatchesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recordedMatchesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QPushButton *watchMatchButton = new QPushButton("🔁 Watch Match", this);
    connect(watchMatchButton, &QPushButton::clicked, this, &TicTacToe::loadRecordedMatch);

    QPushButton *backToModeButtonRecorded = new QPushButton("Back", this);
    connect(backToModeButtonRecorded, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });

    recordedLayout->addWidget(recordedTitle);
    recordedLayout->addWidget(recordedMatchesTable);
    recordedLayout->addWidget(watchMatchButton);
    recordedLayout->addWidget(backToModeButtonRecorded);

    stackedWidget->addWidget(recordedMatchesWidget);

    // === Set Central Widget ===
    setCentralWidget(stackedWidget);
    setWindowTitle("Tic Tac Toe");
}
