#include "battlefield.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QNetworkReply>


BattleField::BattleField(QWidget *parent) : QWidget(parent), manager(new QNetworkAccessManager(this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    gameFormatCombo = new QComboBox(this);
    gameFormatCombo->addItem("User vs User");
    gameFormatCombo->addItem("User vs Computer");
    layout->addWidget(gameFormatCombo);

    gameListCombo = new QComboBox(this);
    layout->addWidget(gameListCombo);


    QPushButton *createGameButton = new QPushButton("Создать игру", this);
    layout->addWidget(createGameButton);
    connect(createGameButton, &QPushButton::clicked, this, &BattleField::createNewGame);

    QPushButton *selectButton = new QPushButton("Выбрать игру", this);
    layout->addWidget(selectButton);
    connect(selectButton, &QPushButton::clicked, this, [this] {
        if (!gameListCombo->currentText().isEmpty()) {
            gameId = gameListCombo->currentText();
            qDebug() << gameId;
            QMessageBox::information(this, "Game Selected", "ID: " + gameId);
        } else {
            QMessageBox::warning(this, "Warning", "Выберите игру!");
        }
    });

    QPushButton *placeShipsButton = new QPushButton("Расставить корабли", this);
    layout->addWidget(placeShipsButton);
    connect(placeShipsButton, &QPushButton::clicked, this, &BattleField::submitShips);


    startGameButton = new QPushButton("Начать игру", this);
    layout->addWidget(startGameButton);
    connect(startGameButton, &QPushButton::clicked, this, &BattleField::startGame);



    QHBoxLayout *fieldsLayout = new QHBoxLayout();


    QVBoxLayout *playerFieldLayout = new QVBoxLayout();
    QLabel *playerLabel = new QLabel("Ваше поле", this);
    playerFieldLayout->addWidget(playerLabel);

    QGroupBox *playerFieldBox = new QGroupBox(this);
    playerGrid = new QGridLayout();
    playerFieldBox->setLayout(playerGrid);
    playerFieldBox->setStyleSheet("border: 2px solid black;");
    playerFieldLayout->addWidget(playerFieldBox);

    fieldsLayout->addLayout(playerFieldLayout);

    QVBoxLayout *opponentFieldLayout = new QVBoxLayout();
    QLabel *opponentLabel = new QLabel("Поле противника", this);
    opponentFieldLayout->addWidget(opponentLabel);

    QGroupBox *opponentFieldBox = new QGroupBox(this);
    opponentGrid = new QGridLayout();
    opponentFieldBox->setLayout(opponentGrid);
    opponentFieldBox->setStyleSheet("border: 2px solid black;");
    opponentFieldLayout->addWidget(opponentFieldBox);

    fieldsLayout->addLayout(opponentFieldLayout);

    layout->addLayout(fieldsLayout);


    setupFields();

    fetchAvailableGames();
}


void BattleField::onOpponentCellClicked(int row, int col) {


    qDebug() << "Удар по X: " + QString::number(row) + " Y: " + QString::number(col);
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    button->setStyleSheet("background-color: blue;");
    sendCoordinates(row, col);
}


void BattleField::startGame() {
    gameType = (gameFormatCombo->currentText() == "User vs Computer") ? "pvc" : "pvp";

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            QPushButton *cell = opponentCells[row * 10 + col];
            connect(cell, &QPushButton::clicked, this, [this, row, col]() {
                onOpponentCellClicked(row, col);
            });
        }
    }

    sendGameStartData();
}



void BattleField::sendGameStartData() {
    if (gameId.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "ID игры не выбран!");
        return;
    }

    QUrl url("http://localhost:8000/api/start_game?game_id=" + gameId + "&format=" + gameType);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (manager) {
        delete manager;
    }
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
      //      QMessageBox::information(this, "Game Started", "Игра успешно начата!");
        } else {
            QMessageBox::critical(this, "Error", "Ошибка старта игры.");
        }
        reply->deleteLater();
    });

    manager->get(request);
}



void BattleField::sendCoordinates(int row, int col) {
    QUrl url("http://localhost:8000/api/attack");
    QNetworkRequest request(url);


    QJsonObject attackData;
    attackData["row"] = row;
    attackData["col"] = col;
    attackData["game_id"] = gameId;
    attackData["player"] = "player1";


    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");


    manager->post(request, QJsonDocument(attackData).toJson());
    qDebug() << attackData;

    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        handleAttackResponse(reply);
    });
}





void BattleField::setupFields() {

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            QPushButton *playerCell = new QPushButton(this);
            playerCell->setFixedSize(40, 40);
            playerCell->setStyleSheet("border: 1px solid black;");
            playerGrid->addWidget(playerCell, row, col);
            playerCells.append(playerCell);

            connect(playerCell, &QPushButton::clicked, this, [this, row, col]() {
                placeShip(row, col);
            });
        }
    }
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            QPushButton *opponentCell = new QPushButton(this);
            opponentCell->setFixedSize(40, 40);
            opponentCell->setStyleSheet("border: 1px solid black;");
            opponentGrid->addWidget(opponentCell, row, col);
            opponentCells.append(opponentCell);

            connect(opponentCell, &QPushButton::clicked, this, [this, row, col]() {
                sendCoordinates(row, col);
            });
        }
    }
}

void BattleField::fetchAvailableGames() {
    QUrl url("http://localhost:8000/api/available_games");
    QNetworkRequest request(url);

    if (manager){
        delete manager;
    }

    manager = new QNetworkAccessManager(this);

    manager->get(request);

    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response_data = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);
            QJsonArray gamesArray = jsonResponse["games"].toArray();

            gameListCombo->clear();
            for (const QJsonValue &gameValue : gamesArray) {
                QString gameId = gameValue.toString();
                gameListCombo->addItem(gameId);
            }
        }
        reply->deleteLater();
    });
}


void BattleField::createNewGame() {
    QString format = gameFormatCombo->currentText();
    QString gameFormatParam = (format == "User vs User") ? "pvp" : "pvc";

    gameType = gameFormatParam;

    QUrl url("http://localhost:8000/api/create_game?format=" + gameFormatParam);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (manager){
        delete manager;
    }

    manager = new QNetworkAccessManager(this);

    manager->post(request, QByteArray());

    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject response = doc.object();
            gameId = response["game_id"].toString();
            qDebug() << "Создана новая игра с ID:" << gameId;


            gameListCombo->addItem(gameId);
            gameListCombo->setCurrentText(gameId);


         //   fetchAvailableGames();

            QMessageBox::information(this, "Игра создана", "Новая игра создана! Game ID: " + gameId);
            qDebug() << gameId;
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка создания игры.");
        }
        reply->deleteLater();
    });
}






void BattleField::placeShip(int row, int col) {
    playerShips.append(qMakePair(row, col));
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    button->setStyleSheet("background-color: gray;");
}

void BattleField::submitShips() {
    QUrl url("http://localhost:8000/api/place_ships?game_id=" + gameId + "&player=player1");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    QJsonArray shipsArray;

    for (const auto &ship : playerShips) {
        QJsonArray shipCoords;
        shipCoords.append(ship.first);
        shipCoords.append(ship.second);
        shipsArray.append(shipCoords);
    }

    json["ships"] = shipsArray;

    if (manager){
        delete manager;

    }
    manager = new QNetworkAccessManager(this);

    manager->post(request, QJsonDocument(json).toJson());

    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Ships Placed", "Ваши корабли успешно расставлены! Можем начинать!");
            return;
        } else {
            QMessageBox::critical(this, "Error", "Не получилось расставить Ваши корабли!");
        }
    });
}


void BattleField::handleAttackResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response_data = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);
        QJsonObject responseObject = jsonResponse.object();

        qDebug() << responseObject;

        int row = responseObject["row"].toInt();
        int col = responseObject["col"].toInt();
        QString result = responseObject["result"].toString();

        updateOpponentCellAppearance(row, col, result);

        if (gameType == "pvc" && responseObject.contains("ai_result")) {
            int aiRow = responseObject["ai_row"].toInt();
            int aiCol = responseObject["ai_col"].toInt();
            QString aiResult = responseObject["ai_result"].toString();
            qDebug() << aiRow;

            updatePlayerField(QJsonArray({ QJsonArray({ aiRow, aiCol, aiResult }) }));
        }

        if (responseObject["victory"].toBool()) {
            QMessageBox::information(this, "Game Over", "Победа!");
            endGame();
        }

    } else {
        QMessageBox::critical(this, "Error", "Ошибка атаки");
    }
    reply->deleteLater();
}


void BattleField::updateOpponentCellAppearance(int row, int col, const QString &result) {
    QPushButton *cell = qobject_cast<QPushButton*>(opponentGrid->itemAtPosition(row, col)->widget());

    if (result == "hit") {
        cell->setStyleSheet("background-color: red;");
    } else if (result == "miss") {
        cell->setStyleSheet("background-color: blue;");
    }
}

void BattleField::updatePlayerField(const QJsonArray &ships) {
    for (const QJsonValue &ship : ships) {
        QJsonArray shipData = ship.toArray();
        int row = shipData[0].toInt();
        int col = shipData[1].toInt();
        QString result = shipData[2].toString();

        QPushButton *cell = qobject_cast<QPushButton*>(playerGrid->itemAtPosition(row, col)->widget());

        if (result == "hit") {
            cell->setStyleSheet("background-color: red;");
        } else if (result == "miss") {
            cell->setStyleSheet("background-color: green;");
        }
    }
}

void BattleField::endGame() {
    QMessageBox::information(this, "Game Over", "Игра окончена!");
}
