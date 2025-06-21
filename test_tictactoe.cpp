#include <QtTest>
#include "tictactoe.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlDriver>

class TestTicTacToe : public QObject
{
    Q_OBJECT

public:
    TestTicTacToe();
    ~TestTicTacToe();

private slots:
    // Special setup and cleanup functions
    void initTestCase();    // Runs once before all tests
    void cleanupTestCase(); // Runs once after all tests
    void init(); // This will reset the game state for us
//....................................................................
    // Test that game basics
    void testInitialState();
    void testWinCondition();
    void testWinCondition_Diagonal();
    void testDrawCondition();
    void testInvalidMoveIsIgnored();

    // Test AI
    void testAiMakesBlockingMove();
    void testAiMakesWinningMove();

    // Test PVP
    void testPlayerVsPlayerFlow();

    // Test the sereies logic ( games set to win ) is working well
    void testSeriesEndLogic();

    //Test Database
    void testUserRegistrationInDatabase();
    void testGameSaveToDatabase();

    // Test Login settings
    void testGuestLogin();
    void testSuccessfulRegistration();
    void testFailedRegistration_WhenUserExists();
    void testSuccessfulLogin();
    void testFailedLogin_WithWrongPassword();

    // Test Themes
    void testThemeApplication();

    // Ai resposnse
    void testPerformance_HardAiMove() ;

    // Integration tests
    void testIntegration_ButtonClick();
    void testIntegration_GameSettings();
    void testIntegration_RegistrationAndLoginFlow();
    void testMatchHistoryPopulation();
    void testGameSettingsIntegration();
private:
    TicTacToe *game; // We will use a pointer to the game object
};

// --- Implementation ---

TestTicTacToe::TestTicTacToe() : game(nullptr) {}
TestTicTacToe::~TestTicTacToe() {}

void TestTicTacToe::initTestCase()
{
    QSqlDatabase testDb = QSqlDatabase::addDatabase("QSQLITE", "test_connection");
    testDb.setDatabaseName(":memory:");
    QVERIFY(testDb.open());
    game = new TicTacToe();
    game->isTestRun = true;
    game->db = testDb;
    game->createTablesIfNeeded();
    qDebug() << "Test Database and Game object created.";
}

void TestTicTacToe::cleanupTestCase()
{delete game;
game = nullptr;
    QSqlDatabase::removeDatabase("test_connection");
    qDebug() << "Game object and Test Database cleaned up.";
}

void TestTicTacToe::init()
{
    game->resetGame();
    game->player1Name = "TestPlayerX";
    game->player2Name = "TestPlayerO";
    QSqlQuery clearQuery(game->db);
    clearQuery.exec("DELETE FROM users");
    clearQuery.exec("DELETE FROM matches");
}

void TestTicTacToe::testInitialState()
{
    for (int i = 0; i < 9; ++i) {
        QCOMPARE(game->getBoardState(i / 3, i % 3), ' ');
    }
}

void TestTicTacToe::testWinCondition()
{
    char startingPlayer = game->getCurrentPlayer();
    game->makeMove(0); game->makeMove(4);
    game->makeMove(1); game->makeMove(5);
    game->makeMove(2); // Winning move
    QVERIFY(game->checkWin(startingPlayer, game->board)); //
}

void TestTicTacToe::testWinCondition_Diagonal()
{
    char winningPlayer = game->getCurrentPlayer();
    game->makeMove(0); // P1
    game->makeMove(1); // P2
    game->makeMove(4); // P1
    game->makeMove(2); // P2
    game->makeMove(8); // P1

       QVERIFY(game->checkWin(winningPlayer, game->board));
}

void TestTicTacToe::testDrawCondition()
{
    game->makeMove(0); game->makeMove(2);
    game->makeMove(1); game->makeMove(3);
    game->makeMove(4); game->makeMove(7);
    game->makeMove(5); game->makeMove(8);
    game->makeMove(6); // Final move for a draw
     QVERIFY(game->checkTie(game->board));
}

void TestTicTacToe::testInvalidMoveIsIgnored()
{
    char playerWhoMoved = game->getCurrentPlayer();
    game->makeMove(4);
    char nextPlayer = game->getCurrentPlayer();
    game->makeMove(4);
    QCOMPARE(game->getBoardState(1, 1), playerWhoMoved);
    QCOMPARE(game->getCurrentPlayer(), nextPlayer);
}

void TestTicTacToe::testAiMakesBlockingMove()
{
    game->difficulty = 3;
    game->mode = 2;
    std::vector<char> setup = { 'O', 'O', ' ', ' ', 'X', ' ', ' ', ' ', 'X' };
    game->setTestBoardState(setup, 'X');
    game->firstMoveMade = true;
    game->makeAIMove();
    QCOMPARE(game->getBoardState(0, 2), 'X');
}

void TestTicTacToe::testAiMakesWinningMove()
{
    game->difficulty = 3;
    game->mode = 2;
    std::vector<char> setup = { 'X', 'X', ' ',  // <-- AI needs to play here
                               'O', 'O', ' ',
                               ' ', ' ', ' ' };
    game->setTestBoardState(setup, 'X');
    game->firstMoveMade = true; // Ensure the AI uses its main logic.
    game->makeAIMove();
    QCOMPARE(game->getBoardState(0, 2), 'X');
}

void TestTicTacToe::testSeriesEndLogic()
{
    game->totalGames = 3;
    game->gamesToWin = 2;
    game->player1Wins = 1;
    game->player2Wins = 0;
    game->resetGame();
    QCOMPARE(game->getBoardState(0, 0), ' '); // Check one square is enough
    game->player1Wins = 2;
    std::vector<char> finalBoardState = { 'X', 'X', 'X', 'O', 'O', ' ', ' ', ' ', ' ' };
    game->setTestBoardState(finalBoardState, 'O');
    game->resetGame();
    QCOMPARE(game->getBoardState(0, 0), 'X');
}

void TestTicTacToe::testUserRegistrationInDatabase()
{
    game->usernameEdit->setText("testuser");
    game->passwordEdit->setText("password123");
    game->registerAccount();
    QSqlQuery query(game->db);
    query.exec("SELECT username FROM users WHERE username = 'testuser'");
    QVERIFY(query.next()); // Verify that we found the result
    QCOMPARE(query.value(0).toString(), "testuser"); // Compare the found username
}



void TestTicTacToe::testGameSaveToDatabase()
{
    game->player1Name = "PlayerOne";
    game->player2Name = "PlayerTwo";
    game->moveHistory = {0, 3, 1, 4, 2}; // A sample move history
    game->gameStartingPlayer = 'X'; // Set a starting player
    game->saveIndividualGameWithNumber("PlayerOne", "Win", 1);

    QSqlQuery query(game->db);
    
    query.exec("SELECT winner, result, moves FROM matches WHERE player1 = 'PlayerOne'");
    QVERIFY(query.next()); // Verify we found the saved match
    QCOMPARE(query.value(0).toString(), "PlayerOne");
    QCOMPARE(query.value(1).toString(), "Win");
    QCOMPARE(query.value(2).toString(), "0,3,1,4,2");
}
void TestTicTacToe::testGuestLogin()
{
    game->guestLogin();
    QVERIFY(game->guestMode == true);
    QCOMPARE(game->loggedInUser, QString("Player 1"));
    QCOMPARE(game->stackedWidget->currentIndex(), 1); // Should be on the mode selection screen
}

void TestTicTacToe::testSuccessfulRegistration()
{
    game->usernameEdit->setText("new_user");
    game->passwordEdit->setText("password123");
    game->registerAccount();
    QSqlQuery query(game->db);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = :user");
    query.bindValue(":user", "new_user");
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1); // We expect to find exactly 1 user with that name.
}

void TestTicTacToe::testFailedRegistration_WhenUserExists()
{
    game->usernameEdit->setText("existing_user");
    game->passwordEdit->setText("password123");
    game->registerAccount(); // The user "existing_user" is now in the database.
    game->registerAccount();
    QSqlQuery query(game->db);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = :existing_user");
    query.bindValue(":existing_user", "existing_user");
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1); // The count should still be 1.
}

void TestTicTacToe::testSuccessfulLogin()
{
    game->usernameEdit->setText("login_user");
    game->passwordEdit->setText("good_password");
    game->registerAccount();
    game->usernameEdit->setText("login_user");
    game->passwordEdit->setText("good_password");
    game->handleLogin();
    QCOMPARE(game->loggedInUser, QString("login_user"));
    QVERIFY(game->guestMode == false);
    QCOMPARE(game->stackedWidget->currentIndex(), 1); // Should be on the mode selection screen.
}

void TestTicTacToe::testFailedLogin_WithWrongPassword()
{
    game->usernameEdit->setText("login_user");
    game->passwordEdit->setText("good_password");
    game->registerAccount();
    QString userBeforeLogin = game->loggedInUser;
    game->usernameEdit->setText("login_user");
    game->passwordEdit->setText("wrong_password");
    game->handleLogin();
    QCOMPARE(game->loggedInUser, userBeforeLogin);
}

void TestTicTacToe::testThemeApplication()
{
    game->selectedTheme = "Dark";
    game->applyStyleSheet();
    QVERIFY(!game->styleSheet().isEmpty());
}

void TestTicTacToe::testPlayerVsPlayerFlow()
{
    game->setPlayerVsPlayer();
    QCOMPARE(game->mode, 1); // Verify mode was set.
    char startingPlayer = game->getCurrentPlayer();
    int p1ScoreBefore = game->player1Wins;
    int p2ScoreBefore = game->player2Wins;
    game->makeMove(0); // P1
    game->makeMove(3); // P2
    game->makeMove(1); // P1
    game->makeMove(4); // P2
    game->makeMove(2); // P1 wins
    if (startingPlayer == game->PLAYER1) {
        QCOMPARE(game->player1Wins, p1ScoreBefore + 1);
    } else {
        QCOMPARE(game->player2Wins, p2ScoreBefore + 1);
    }
}

void TestTicTacToe::testPerformance_HardAiMove()
{
    game->difficulty = 3;
    game->mode = 2;
    game->firstMoveMade = true;
    std::vector<char> boardState = { 'X', ' ', ' ',
                                    ' ', 'O', ' ',
                                    ' ', ' ', ' ' };
    game->setTestBoardState(boardState, 'X'); // Set it to AI's turn
    QElapsedTimer timer;
    timer.start();
    game->makeAIMove();
    qint64 elapsedNanoseconds = timer.nsecsElapsed();
    qint64 elapsedMicroseconds = elapsedNanoseconds / 1000; // Convert nanoseconds to microseconds
    qDebug() << "AI move performance benchmark:" << elapsedMicroseconds << "us.";
    QVERIFY(elapsedMicroseconds < 2000000);
}

void TestTicTacToe::testIntegration_ButtonClick()
{
    game->setPlayerVsPlayer();
    char startingPlayer = game->getCurrentPlayer();
    QPushButton* topLeftButton = game->findChild<QPushButton*>("gameButton_0");
    QVERIFY(topLeftButton != nullptr);
    QCOMPARE(topLeftButton->text(), "");
    QTest::mouseClick(topLeftButton, Qt::LeftButton);
    QCOMPARE(topLeftButton->text(), QString(startingPlayer));
    QCOMPARE(game->getBoardState(0, 0), startingPlayer);
}

void TestTicTacToe::testIntegration_GameSettings()
{
    QSpinBox* totalGames = game->findChild<QSpinBox*>("totalGamesSpinBox");
    QSpinBox* gamesToWin = game->findChild<QSpinBox*>("gamesToWinSpinBox");
    QPushButton* applyButton = game->findChild<QPushButton*>("applySettingsButton");
    QVERIFY(totalGames && gamesToWin && applyButton); // Verify all widgets were found.
    totalGames->setValue(7);
    gamesToWin->setValue(4);
    QTest::mouseClick(applyButton, Qt::LeftButton);
    QCOMPARE(game->totalGames, 7);
    QCOMPARE(game->gamesToWin, 4);
}

void TestTicTacToe::testIntegration_RegistrationAndLoginFlow()
{
    QLineEdit* username = game->findChild<QLineEdit*>("usernameLineEdit");
    QLineEdit* password = game->findChild<QLineEdit*>("passwordLineEdit");
    QPushButton* registerBtn = game->findChild<QPushButton*>("registerButton");
    QPushButton* loginBtn = game->findChild<QPushButton*>("loginButton");
    QVERIFY(username && password && registerBtn && loginBtn);
    username->setText("integration_user");
    password->setText("pass123");
    QTest::mouseClick(registerBtn, Qt::LeftButton);
    QSqlQuery query(game->db);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = 'integration_user'");
    QVERIFY(query.exec() && query.next());
    QCOMPARE(query.value(0).toInt(), 1);
    username->setText("integration_user");
    password->setText("pass123");
    QTest::mouseClick(loginBtn, Qt::LeftButton);
    QCOMPARE(game->loggedInUser, QString("integration_user"));
    QCOMPARE(game->stackedWidget->currentIndex(), 1);
}

void TestTicTacToe::testMatchHistoryPopulation()
{
    game->loggedInUser = "history_user";
    game->player1Name = "history_user";
    game->player2Name = "AI";

    game->saveIndividualGameWithNumber("history_user", "Win", 1);
    game->saveIndividualGameWithNumber("AI", "Defeat", 2);
    game->saveIndividualGameWithNumber("history_user", "Win", 3);

    QPushButton* historyButton = game->findChild<QPushButton*>("View Match History");
    QVERIFY(historyButton != nullptr); // Make sure we found the button.
    QTest::mouseClick(historyButton, Qt::LeftButton);
    QCOMPARE(game->stackedWidget->currentIndex(), 6);
    QTableWidget* table = game->findChild<QTableWidget*>("matchHistoryTable");
    QVERIFY(table != nullptr); // Make sure we found the table.
    QCOMPARE(table->rowCount(), 3);
}

void TestTicTacToe::testGameSettingsIntegration()
{
    game->setPlayerVsPlayer(); // This slot takes us to the settings screen
    QSpinBox* totalGames = game->findChild<QSpinBox*>("totalGamesSpinBox");
    QSpinBox* gamesToWin = game->findChild<QSpinBox*>("gamesToWinSpinBox");
    QPushButton* applyButton = game->findChild<QPushButton*>("applySettingsButton");
    QVERIFY(totalGames && gamesToWin && applyButton);
    totalGames->setValue(5);
    gamesToWin->setValue(3);
    QTest::mouseClick(applyButton, Qt::LeftButton);
    QCOMPARE(game->totalGames, 5);
    QCOMPARE(game->gamesToWin, 3);
}
QTEST_MAIN(TestTicTacToe)
#include "test_tictactoe.moc"