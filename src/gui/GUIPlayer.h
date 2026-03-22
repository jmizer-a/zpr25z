#ifndef GUIPLAYER_H
#define GUIPLAYER_H

#include <QWidget>
#include "core/Player.h"
#include "gui/MainWindow.h"

// Author: Jerzy Mossakowski
// Implements player interacting with the game via GUI

class GUIPlayer : public Player {
public:
    GUIPlayer( std::string name, MainWindow* parent = nullptr );
    PlayerDecision decideBuyOrAuction( const PurchasableCard& card ) override;
    void handleMortgageToggle( size_t card_index );
    void handleBuilding( size_t card_index );
    void informOfRandomCard( const RandomCard& card ) override;
    unsigned int decideBid( const PurchasableCard& card, unsigned int current_bid ) override;
    bool useGetOutOfJailCard() override;
    DiceResult takeTurn() override;
    DiceResult rollDice() const override;
    bool wantsToTrade() override;
    TradeOffer tradeOffer() override;
    bool decideOnTrade( TradeOffer offer ) override;

private:
    MainWindow* parentWindow_;
};

#endif  // GUIPLAYER_H
