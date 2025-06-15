#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlDatabase>   // SQL
#include <QSqlQuery>      // SQL
#include <vector>
#include <QComboBox>
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
#include <QObject> // Added for QOverload
#include <QOverload>
#include <QCryptographicHash>
#include <QByteArray>
#include <QRandomGenerator>

class TicTacToe : public QMainWindow {
    Q_OBJECT;

public:
    explicit TicTacToe(QWidget *parent = nullptr);

private slots:
    void handleButtonClick(int index);
    void setPlayerVsPlayer();
    void setPlayerVsAI();
    void setDifficultyEasy();
    void setDifficultyMedium();
    void setDifficultyHard();
    void handleLogin();
    void registerAccount();    // 🔥 Register a new account
    void guestLogin();         // 🔥 Play as guest
    void startPvPWithNames();
    void applyGameSettings();
    void logout();
    void backToModeSelection();
    void toggleScoreboard();
    void handleSurrender();
    void deleteAccount();
private:
    // Constants
    static constexpr char PLAYER1 = 'X';
    static constexpr char PLAYER2 = 'O';
    static constexpr char EMPTY = ' ';

    // Game state
    int surrenderCount = 0;
    int mode = 1; // 1: PvP, 2: PvAI
    int difficulty = 3; // 1: Easy, 2: Medium, 3: Hard
    char currentPlayer = PLAYER2;
    std::vector<char> board;
    bool firstMoveMade = false;
    bool nightMode = false;
    bool scoreboardVisible = false;
    int totalGames = 3;
    int gamesToWin = 2;
    int player1Wins = 0;
    int player2Wins = 0;
    int ties = 0;
    QString currentSeriesId; // Track current series
    QString player1Name;
    QString player2Name;
    QString loggedInUser;
    struct OriginalGameState {
        QString loggedInUser;
        int player1Wins;
        int player2Wins;
        int ties;
        char currentPlayer;
    } originalGameState;
    bool guestMode = false; // 🔥 New
    QString replayGameMode; // Track original game mode during replay
    std::vector<int> moveHistory; // Stores the history of moves
    std::vector<int> replayMoves;
    int replayIndex;
    char replayStartingPlayer;
    char gameStartingPlayer = PLAYER1; // Store starting player when game begins
    bool inReplayMode = false;
    // UI elements
    QButtonGroup *buttonGroup;
    QLabel *statusLabel;
    QStackedWidget *stackedWidget;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *player2NameEdit;
    QSpinBox *totalGamesSpinBox;
    QSpinBox *gamesToWinSpinBox;
    QLabel *scoreLabel;
    QPushButton *nightModeButton;
    QPushButton *scoreboardToggleButton;
    QTextEdit *matchHistoryTextEdit;
    QTableWidget *matchHistoryTable;
    QString selectedTheme = "Light"; // Default theme
    QComboBox *themeSelector;
    QTableWidget *recordedMatchesTable;
    // Database
    QSqlDatabase db;
    // DB Methods
    void connectToDatabase();
    void createTablesIfNeeded();
    void saveMatchResult(const QString &winner, const QString &result);
    void loadMatchHistory();
    QString pbkdf2Hash(const QString &password, const QByteArray &salt, int iterations, int dkLen);
    QByteArray generateSalt(int length);
    // Game Methods
    void setupUI();
    void applyStyleSheet();
    void updateScoreboard();
    void resetGame();
    void updateBoard();
    void updateStatus();
    void makeMove(int index);
    void gameOver(const QString &message, bool seriesOver = false);
    void makeAIMove();
    void easyMove();
    void mediumMove();
    void hardMove();
    int minimax(int depth, bool isMaximizing);
    bool checkWin(char player);
    bool checkTie();
    bool isMatchUnfinished();
    void loadRecordedMatchesScreen();
    void loadRecordedMatch();
    void replayNextMove();
    void saveIndividualGame(const QString &winner, const QString &result);
    void handleSeriesEnd();
};

#endif // TICTACTOE_H

