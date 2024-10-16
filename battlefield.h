#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QComboBox>
#include <QMessageBox>

class BattleField : public QWidget {
    Q_OBJECT

public:
    BattleField(QWidget *parent = nullptr);

signals:
    void coordinateSelected(int row, int col);

private slots:
    void onOpponentCellClicked(int row, int col);
    void startGame();

private:
    QGridLayout *playerGrid;
    QGridLayout *opponentGrid;
    QComboBox *gameListCombo;
    QComboBox *gameFormatCombo;
    QPushButton *startGameButton;
    QString gameId;
    QNetworkAccessManager *manager;
    QVector<QPushButton*> playerCells;
    QVector<QPushButton*> opponentCells;
    QString gameType;

    void fetchAvailableGames();
    void selectGame();
    void setupFields();
    void sendCoordinates(int row, int col);
    void handleAttackResponse(QNetworkReply *reply);
    void updateOpponentCellAppearance(int row, int col, const QString &result);
    void updatePlayerField(const QJsonArray &ships);
    void endGame();
    void placeShip(int row, int col);
    void submitShips();
    QVector<QPair<int, int>> playerShips;
    void createNewGame();
    void sendGameStartData();
};

#endif // BATTLEFIELD_H
