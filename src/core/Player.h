#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Card.h"
#include "Property.h"
#include "Space.h"

const int STARTING_CASH = 1500;

// Author: Jerzy Mossakowski
// Implements a player in the Monopoly game with deck and resources. Inheriting classes represent interfaces and bot
// players.

class Game;

enum class PlayerDecision : std::uint8_t { BUY, AUCTION };

struct TradeOffer {
    Player* requester_ = nullptr;
    Player* receiver_ = nullptr;
    unsigned int cashOffered_ = 0;
    PurchasableCard* cardOffered_ = nullptr;
    unsigned int cashRequested_ = 0;
    PurchasableCard* cardRequested_ = nullptr;
};

struct DiceResult {
    unsigned int die1_;
    unsigned int die2_;
    unsigned int total() const { return die1_ + die2_; }
    bool isDouble() const { return die1_ == die2_; }
};

class Player {
public:
    Player( std::string name ) : name_( std::move( name ) ){};
    virtual ~Player() = default;

    virtual PlayerDecision decideBuyOrAuction( const PurchasableCard& card ) = 0;
    virtual void informOfRandomCard( const RandomCard& card ) = 0;
    virtual unsigned int decideBid( const PurchasableCard& card, unsigned int current_bid ) = 0;
    virtual bool useGetOutOfJailCard() = 0;
    virtual DiceResult takeTurn() = 0;
    virtual DiceResult rollDice() const = 0;
    virtual bool wantsToTrade() = 0;
    virtual TradeOffer tradeOffer() = 0;
    virtual bool decideOnTrade( TradeOffer trade_offer ) = 0;

    void build( Property& property_card );
    void addCash( unsigned int amount );
    void addOwnedCard( Card* card );
    void addJailFreeCard();
    void removeOwnedCard( const std::string& name );
    void subtractCash( unsigned int amount );
    void setTurnsInJail( unsigned int turns );
    void payRepairCosts( unsigned int cost_per_house, unsigned int cost_per_hotel );

    void setGameContext( const std::vector<std::unique_ptr<Player>>& players,
                         const std::vector<std::unique_ptr<Space>>& board,
                         const std::unordered_map<Player*, unsigned int>& player_positions ) {
        players_ = &players;
        board_ = &board;
        playerPositions_ = &player_positions;
    }

    const std::string& getName() const;
    unsigned int getCash() const;
    const std::vector<Card*>& getOwnedCards() const;
    unsigned int getTurnsInJail() const;
    bool isLost() const;

    bool canSellBuilding( Property* property_card ) const;

    void mortgageCard( Card& card );
    void unmortgageCard( Card& card );
    void lose();

protected:
    void forcedMortgage( const unsigned int amount );
    void forcedSell( const unsigned int amount );
    std::vector<Property*> findColorGroup( Property& property_card );
    int countCardsInColorGroup( Property& property_card );
    bool isOtherPropertyUndeveloped( std::vector<Property*> group, const int level );
    bool isMonopoly( Property& property_card );

    const std::string name_;
    unsigned int cash_ = STARTING_CASH;
    unsigned int turnsInJail_ = 0;
    std::vector<Card*> ownedCards_;
    bool lost_ = false;
    const std::vector<std::unique_ptr<Player>>* players_ = nullptr;
    const std::vector<std::unique_ptr<Space>>* board_ = nullptr;
    const std::unordered_map<Player*, unsigned int>* playerPositions_ = nullptr;
};

#endif  // PLAYER_H
