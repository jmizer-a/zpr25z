#ifndef GAME_H
#define GAME_H

#include <memory>
#include <unordered_map>
#include <vector>
#include "Card.h"
#include "Player.h"
#include "Property.h"
#include "Space.h"
#include "SpecialProperties.h"

// Author: Jerzy Mossakowski
// Implements state and logic of the Monopoly game.

class Game {
public:
    Game( std::vector<std::unique_ptr<Player>> players,
          std::vector<std::unique_ptr<Space>> board,
          std::vector<std::unique_ptr<Card>> deck,
          std::vector<std::unique_ptr<RandomCard>> chance_deck,
          std::vector<std::unique_ptr<RandomCard>> community_chest_deck );

    void run();

    void changeActivePlayer();
    void runTurn();

    void payEachPlayer( Player* payer, unsigned int amount );
    void collectFromEachPlayer( Player* collector, unsigned int amount );

    void movePlayer( Player* const player, unsigned int steps );
    void movePlayerToJail( Player* const player );
    void movePlayerToSpace( Player* const player, const std::string& space_name );
    void movePlayerToNearest( Player* const player, SpaceType type );

    void runSpaceAction( Player* const player, const Space& space );
    void runAuction( PurchasableCard* const card );
    void runTrade( Player* const seller, TradeOffer trade_offer );
    void tradeTime();
    void handleRandomCard( Player* const player, SpaceType type );

    Player* getActivePlayer() const;
    unsigned int getLastDiceRoll() const;
    const std::unordered_map<Player*, unsigned int>& getPlayerPositions() const;
    const std::vector<std::unique_ptr<Player>>& getPlayers() const;
    const std::vector<std::unique_ptr<Space>>& getBoard() const;

    Player* getWinner() const;

private:
    std::unordered_map<Player*, unsigned int> playerPositions_;
    Player* activePlayer_ = nullptr;
    unsigned int lastDiceRoll_ = 0;

    const std::vector<std::unique_ptr<Player>> players_;
    const std::vector<std::unique_ptr<Space>> board_;
    const std::vector<std::unique_ptr<Card>> deck_;
    std::vector<std::unique_ptr<RandomCard>> chanceDeck_;
    std::vector<std::unique_ptr<RandomCard>> communityChestDeck_;

    void resolvePropertySpace( Player* const player, PurchasableCard* const card );
};

#endif  // GAME_H
