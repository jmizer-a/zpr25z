#include "Game.h"
#include <algorithm>
#include <cstdlib>

using namespace std;

const int MAX_TURNS = 400;
const int MAX_JAIL_TIME = 3;
const int TAX_AMOUNT = 200;
const int GO_CASH_BONUS = 200;
const string GET_OUT_OF_JAIL_CARD = "Ucieczka z więzienia";

// Author: Jerzy Mossakowski
// Implements main game mechanics, flow and state, along with turn operations, player actions, game state alteration
// and interactions between players and game elements.

// Construct and run Monopoly game_____________________________________________________________________________________

Game::Game( vector<unique_ptr<Player>> players,
            vector<unique_ptr<Space>> board,
            vector<unique_ptr<Card>> deck,
            vector<unique_ptr<RandomCard>> chance_deck,
            vector<unique_ptr<RandomCard>> community_chest_deck )
    : players_( move( players ) ),
      board_( move( board ) ),
      deck_( move( deck ) ),
      chanceDeck_( move( chance_deck ) ),
      communityChestDeck_( move( community_chest_deck ) ) {
    activePlayer_ = players_[0].get();
    for( const auto& player : players_ ) {
        playerPositions_[player.get()] = 0;
        player->setGameContext( players_, board_, playerPositions_ );
    }
}

void Game::run() {
    unsigned int turn_count = 0;
    while( count_if( players_.begin(), players_.end(), []( auto& p ) { return !p->isLost(); } ) > 1 &&
           turn_count < MAX_TURNS ) {
        if( activePlayer_->isLost() ) {
            changeActivePlayer();
            continue;
        }
        runTurn();
        ++turn_count;
    }
}

// Getters_____________________________________________________________________________________________________________

Player* Game::getActivePlayer() const {
    return activePlayer_;
}
unsigned int Game::getLastDiceRoll() const {
    return lastDiceRoll_;
}
const unordered_map<Player*, unsigned int>& Game::getPlayerPositions() const {
    return playerPositions_;
}
const vector<unique_ptr<Player>>& Game::getPlayers() const {
    return players_;
}
const vector<unique_ptr<Space>>& Game::getBoard() const {
    return board_;
}

Player* Game::getWinner() const {
    auto it = find_if( players_.begin(), players_.end(), []( const auto& p ) { return !p->isLost(); } );
    return ( it != players_.end() ) ? it->get() : nullptr;
}

// Turn operations_____________________________________________________________________________________________________

void Game::changeActivePlayer() {
    for( size_t i = 0; i < players_.size(); ++i ) {
        if( players_[i].get() == activePlayer_ ) {
            activePlayer_ = players_[( i + 1 ) % players_.size()].get();
            return;
        }
    }
}

void Game::runTurn() {
    if( activePlayer_->getTurnsInJail() > 0 ) {
        DiceResult result = activePlayer_->takeTurn();
        bool usedCard = false;
        for( auto it = activePlayer_->getOwnedCards().begin(); it != activePlayer_->getOwnedCards().end(); ++it ) {
            if( ( *it )->getName() == GET_OUT_OF_JAIL_CARD ) {
                if( activePlayer_->useGetOutOfJailCard() ) {
                    usedCard = true;
                    activePlayer_->removeOwnedCard( GET_OUT_OF_JAIL_CARD );
                }
                break;
            }
        }
        if( result.isDouble() || usedCard ) {
            activePlayer_->setTurnsInJail( 0 );
            movePlayer( activePlayer_, result.total() );
            runSpaceAction( activePlayer_, *board_[playerPositions_[activePlayer_]] );
        } else {
            activePlayer_->setTurnsInJail( activePlayer_->getTurnsInJail() - 1 );
        }
        tradeTime();
        changeActivePlayer();
        return;
    }
    DiceResult result = activePlayer_->takeTurn();
    lastDiceRoll_ = result.total();
    movePlayer( activePlayer_, result.total() );
    tradeTime();
    changeActivePlayer();
}

// Chance and Chest card money effects_________________________________________________________________________________

void Game::payEachPlayer( Player* payer, const unsigned int amount ) {
    for( auto& p : players_ ) {
        if( p.get() != payer ) {
            payer->subtractCash( amount );
            p->addCash( amount );
        }
    }
}

void Game::collectFromEachPlayer( Player* collector, const unsigned int amount ) {
    for( auto& p : players_ ) {
        if( p.get() != collector ) {
            p->subtractCash( amount );
            collector->addCash( amount );
        }
    }
}

// Move operations_____________________________________________________________________________________________________

void Game::movePlayer( Player* const player, const unsigned int steps ) {
    playerPositions_[player] = ( playerPositions_[player] + steps ) % board_.size();
    if( steps > playerPositions_[player] ) {
        player->addCash( GO_CASH_BONUS );
    }
    runSpaceAction( player, *board_[playerPositions_[player]] );
}

void Game::movePlayerToJail( Player* const player ) {
    for( size_t i = 0; i < board_.size(); ++i ) {
        if( board_[i]->getType() == SpaceType::JAIL ) {
            playerPositions_[player] = i;
            player->setTurnsInJail( MAX_JAIL_TIME );
            return;
        }
    }
}

void Game::movePlayerToSpace( Player* const player, const string& space_name ) {
    for( int i = 0; i < board_.size(); ++i ) {
        if( board_[i]->getName() == space_name ) {
            if( i < playerPositions_[player] ) {
                player->addCash( GO_CASH_BONUS );
            }
            playerPositions_[player] = i;
            runSpaceAction( player, *board_[playerPositions_[player]] );
            return;
        }
    }
}

void Game::movePlayerToNearest( Player* const player, SpaceType type ) {
    for( int i = 0; i < board_.size(); ++i ) {
        int space_index = ( playerPositions_[player] + i ) % board_.size();
        if( board_[space_index]->getType() == type ) {
            playerPositions_[player] = space_index;
            return;
        }
    }
    runSpaceAction( player, *board_[playerPositions_[player]] );
}

// Player move effect operations_______________________________________________________________________________________

void Game::runSpaceAction( Player* const player, const Space& space ) {
    Card* associated = space.getAssociatedCard();
    PurchasableCard* pCard = dynamic_cast<PurchasableCard*>( associated );
    if( pCard ) {
        resolvePropertySpace( player, pCard );
    }
    switch( space.getType() ) {
        case SpaceType::CHANCE:
        case SpaceType::COMMUNITY_CHEST: {
            handleRandomCard( player, space.getType() );
            break;
        }
        case SpaceType::TAX:
            player->subtractCash( TAX_AMOUNT );
            break;
        case SpaceType::GO_TO_JAIL:
            movePlayerToJail( player );
            break;
        case SpaceType::GO:
            break;
    }
}

void Game::handleRandomCard( Player* player, SpaceType type ) {
    auto& deck = ( type == SpaceType::CHANCE ) ? chanceDeck_ : communityChestDeck_;

    size_t index = rand() % deck.size();
    RandomCard* card = deck[index].get();

    player->informOfRandomCard( *card );
    if( card->getName() == GET_OUT_OF_JAIL_CARD ) {
        player->addOwnedCard( card );
        return;
    }
    card->apply( this, player );
}

void Game::resolvePropertySpace( Player* const player, PurchasableCard* const card ) {
    Player* owner = card->getOwner();
    if( owner == nullptr ) {
        PlayerDecision decision = player->decideBuyOrAuction( *card );
        if( decision == PlayerDecision::BUY && player->getCash() >= card->getPrice() ) {
            player->subtractCash( card->getPrice() );
            card->setOwner( player );
            player->addOwnedCard( card );
        } else {
            runAuction( card );
        }
    } else if( owner != player && !card->isMortgaged() ) {
        unsigned int rent = card->calculateRent( this );

        if( player->getCash() < rent ) {
            unsigned int cashHandedOver = player->getCash();
            player->subtractCash( cashHandedOver );
            owner->addCash( cashHandedOver );
        } else {
            player->subtractCash( rent );
            owner->addCash( rent );
        }
    }
    return;
}

// Auctions and trade__________________________________________________________________________________________________

void Game::runAuction( PurchasableCard* const card ) {
    vector<Player*> bidders;
    for( const auto& p : players_ ) {
        bidders.push_back( p.get() );
    }
    unsigned int currentBid = 0;
    Player* winner = nullptr;
    while( bidders.size() > 1 ) {
        auto it = bidders.begin();
        while( it != bidders.end() ) {
            unsigned int bid = ( *it )->decideBid( *card, currentBid );
            if( bid > currentBid ) {
                currentBid = bid;
                winner = *it;
                ++it;
            } else {
                it = bidders.erase( it );
            }
        }
    }
    if( bidders.size() == 1 && winner == nullptr ) {
        unsigned int bid = bidders[0]->decideBid( *card, currentBid );
        if( bid > 0 ) {
            winner = bidders[0];
            currentBid = bid;
        }
    }
    if( winner ) {
        winner->subtractCash( currentBid );
        card->setOwner( winner );
        winner->addOwnedCard( card );
    }
}

void Game::runTrade( Player* const seller, TradeOffer trade_offer ) {
    Player* receiver = trade_offer.receiver_;

    if( !receiver || receiver->isLost() || seller->isLost() )
        return;

    if( trade_offer.cardOffered_ && trade_offer.cardOffered_->getOwner() != seller )
        return;

    if( trade_offer.cardRequested_ && trade_offer.cardRequested_->getOwner() != receiver )
        return;

    if( seller->getCash() < trade_offer.cashOffered_ || receiver->getCash() < trade_offer.cashRequested_ ) {
        return;
    }

    if( trade_offer.cashOffered_ > 0 ) {
        seller->subtractCash( trade_offer.cashOffered_ );
        receiver->addCash( trade_offer.cashOffered_ );
    }

    if( trade_offer.cashRequested_ > 0 ) {
        receiver->subtractCash( trade_offer.cashRequested_ );
        seller->addCash( trade_offer.cashRequested_ );
    }

    if( trade_offer.cardOffered_ ) {
        seller->removeOwnedCard( trade_offer.cardOffered_->getName() );
        receiver->addOwnedCard( trade_offer.cardOffered_ );
        trade_offer.cardOffered_->setOwner( receiver );
    }

    if( trade_offer.cardRequested_ ) {
        receiver->removeOwnedCard( trade_offer.cardRequested_->getName() );
        seller->addOwnedCard( trade_offer.cardRequested_ );
        trade_offer.cardRequested_->setOwner( seller );
    }
}

void Game::tradeTime() {
    if( activePlayer_->wantsToTrade() ) {
        TradeOffer tradeOffer = activePlayer_->tradeOffer();
        if( tradeOffer.receiver_ != nullptr && tradeOffer.receiver_ != activePlayer_ ) {
            if( tradeOffer.receiver_->decideOnTrade( tradeOffer ) )
                runTrade( activePlayer_, tradeOffer );
        }
    }
}
