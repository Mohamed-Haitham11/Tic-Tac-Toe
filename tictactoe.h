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
class TicTacToe : public QMainWindow {
    Q_OBJECT

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
    void toggleNightMode();
    void toggleScoreboard();
private:
    // Constants
    static constexpr char PLAYER1 = 'X';
    static constexpr char PLAYER2 = 'O';
    static constexpr char EMPTY = ' ';

    // Game state
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
    QString player1Name;
    QString player2Name;
    QString loggedInUser;
    bool guestMode = false; // 🔥 New

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
    // Database
    QSqlDatabase db;

    // DB Methods
    void connectToDatabase();
    void createTablesIfNeeded();
    void saveMatchResult(const QString &winner, const QString &result);
    void loadMatchHistory();

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
};

#endif // TICTACTOE_H

