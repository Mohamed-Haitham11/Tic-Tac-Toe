#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QButtonGroup>
#include <QTableWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QSqlError>
#include <QMessageBox>
#include <QTimer>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <vector>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QScreenCapture>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include <QSlider>

class TicTacToe : public QMainWindow
{
    Q_OBJECT

public:
    TicTacToe(QWidget *parent = nullptr);

private slots:
    // Login/Authentication
    void handleLogin();
    void logout();
    void registerAccount();
    void guestLogin();
    void deleteAccount();

    // Game Mode Selection
    void setPlayerVsPlayer();
    void setPlayerVsAI();
    void setDifficultyEasy();
    void setDifficultyMedium();
    void setDifficultyHard();

    // Game Settings
    void applyGameSettings();
    void startPvPWithNames();

    // Game Logic
    void handleButtonClick(int index);
    void handleSurrender();
    void backToModeSelection();

    // UI Updates
    void toggleScoreboard();

    // Match History
    void loadMatchHistory();
    void loadRecordedMatchesScreen();
    void loadRecordedMatch();
    void replayNextMove();

    // Video Recording
    void startVideoRecording();
    void stopVideoRecording();
    void onRecordingFinished();
    void onRecordingError();

    // Video Playback
    void playRecordedVideo();
    void stopVideoPlayback();
    void onVideoPositionChanged(qint64 position);
    void onVideoDurationChanged(qint64 duration);

private:
    // Core Game Constants
    static const char EMPTY   ;
    static const char PLAYER1 ;
    static const char PLAYER2 ;

    // UI Components
    QStackedWidget *stackedWidget;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *player2NameEdit;
    QComboBox *themeSelector;
    QSpinBox *totalGamesSpinBox;
    QSpinBox *gamesToWinSpinBox;
    QButtonGroup *buttonGroup;
    QLabel *statusLabel;
    QLabel *scoreLabel;
    QPushButton *scoreboardToggleButton;
    QTableWidget *matchHistoryTable;
    QTableWidget *recordedMatchesTable;

    // Video Components
    QMediaRecorder *mediaRecorder;
    QMediaCaptureSession *captureSession;
    QScreenCapture *screenCapture;
    QMediaPlayer *videoPlayer;
    QVideoWidget *videoWidget;
    QAudioOutput *audioOutput;
    QPushButton *recordButton;
    QPushButton *playVideoButton;
    QPushButton *stopVideoButton;
    QLabel *videoStatusLabel;
    QLabel *recordingStatusLabel;
    QSlider *videoProgressSlider;

    // Game State
    std::vector<char> board;
    char currentPlayer;
    int mode; // 1 = PvP, 2 = PvE
    int difficulty; // 1 = Easy, 2 = Medium, 3 = Hard
    int totalGames;
    int gamesToWin;
    int player1Wins;
    int player2Wins;
    int ties;
    bool scoreboardVisible;
    bool firstMoveMade;
    bool guestMode;

    // Player Info
    QString loggedInUser;
    QString player1Name;
    QString player2Name;
    QString selectedTheme;

    // Database
    QSqlDatabase db;

    // Replay System
    std::vector<int> moveHistory;
    std::vector<int> replayMoves;
    int replayIndex;
    char replayStartingPlayer;

    // Video Recording
    bool isRecording;
    QString currentVideoPath;
    QString videoDirectory;
    int currentMatchId;

    // Core Functions
    void setupUI();
    void connectToDatabase();
    void createTablesIfNeeded();
    void applyStyleSheet();
    void resetGame();
    void makeMove(int index);
    void makeAIMove();
    int easyMove();
    int mediumMove();
    int hardMove();
    int minimax(std::vector<char> &tempBoard, bool isMaximizing);
    bool checkWin(char player);
    bool checkTie();
    bool isMatchUnfinished();
    void gameOver(const QString &message, bool seriesOver = false);
    void updateBoard();
    void updateStatus();
    void updateScoreboard();
    void saveMatchResult(const QString &winner, const QString &result);
    int minimax(int depth, bool isMaximizing, char aiPlayer, char humanPlayer);
    // Security Functions
    QByteArray generateSalt(int length);
    QString pbkdf2Hash(const QString &password, const QByteArray &salt, int iterations, int dkLen);

    // Video Functions
    void setupVideoRecording();
    void setupVideoPlayback();
    QString generateVideoFileName();
    void updateVideoDatabase(int matchId, const QString &videoPath);
    void createVideoDirectory();
};

#endif // TICTACTOE_H
