#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <functional>
#include <map>
#include <memory>
#include <QEventLoop>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>
#include "core/Player.h"
#include "core/Space.h"

// Author: Jerzy Mossakowski
// Implements the main window where gameplay takes place.

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow( QWidget* parent = nullptr );

    void displayBoard( const std::vector<std::unique_ptr<Space>>& board );
    void updatePlayerStats( const QString& name, unsigned int cash );
    void updateOwnedCards( const std::vector<Card*>& cards );
    void updatePlayerPositions( const std::unordered_map<Player*, unsigned int>& positions );
    void waitForRoll();
    void displayWinner( Player* player );
    void refreshPlayerData( Player* player );
    PlayerDecision askBuyOrAuction( const std::string& name, unsigned int price, unsigned int cash );
    void showRandomCard( const std::string& description );
    unsigned int askBid( const std::string& name, unsigned int current_bid, unsigned int cash );
    bool askUseJailCard();
    bool askWantsToTrade( const std::string& player_name );
    TradeOffer createTradeOffer( Player* requester, const std::vector<std::unique_ptr<Player>>& all_players );
    bool askAcceptTrade( Player* receiver, TradeOffer offer );

    std::function<void( size_t )> onBuildClicked_;
    std::function<void( size_t )> onMortgageClicked_;

private:
    void setupBoardGrid();
    QString getHexColor( Color color );

    QLabel* currentPlayerLabel_;
    QLabel* cashLabel_;
    QPushButton* rollButton_;
    std::vector<QLabel*> spaceLabels_;
    QGridLayout* boardLayout_;

    QVBoxLayout* cardsLayout_;
    QWidget* cardsScrollWidget_;

    std::map<Player*, QLabel*> playerPawns_;

    QVBoxLayout* playerListLayout_;
    std::map<Player*, QVBoxLayout*> playerSectionMap_;
};

#endif  // MAINWINDOW_H
