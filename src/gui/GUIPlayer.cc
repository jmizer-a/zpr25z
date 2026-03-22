#include "GUIPlayer.h"
#include <algorithm>
#include <ctime>
#include "core/Property.h"
#include "core/SpecialProperties.h"
#include "MainWindow.h"

using namespace std;

// Author: Jerzy Mossakowski
// Implements Player interface for a GUI-based player using Qt for interaction, as class inhereting from generic Player

// Setup and initialisation____________________________________________________________________________________________

GUIPlayer::GUIPlayer( string name, MainWindow* parent ) : Player( move( name ) ), parentWindow_( parent ) {
    srand( static_cast<unsigned int>( time( nullptr ) ) );
}

DiceResult GUIPlayer::takeTurn() {
    parentWindow_->refreshPlayerData( this );

    if( board_ ) {
        parentWindow_->displayBoard( *board_ );
    }
    if( playerPositions_ ) {
        parentWindow_->updatePlayerPositions( *playerPositions_ );
    }

    parentWindow_->onBuildClicked_ = [this]( size_t index ) {
        this->handleBuilding( index );
        parentWindow_->refreshPlayerData( this );
    };

    parentWindow_->onMortgageClicked_ = [this]( size_t index ) {
        this->handleMortgageToggle( index );
        parentWindow_->refreshPlayerData( this );
    };

    parentWindow_->waitForRoll();

    parentWindow_->onBuildClicked_ = nullptr;
    parentWindow_->onMortgageClicked_ = nullptr;

    return rollDice();
}

DiceResult GUIPlayer::rollDice() const {
    DiceResult result;
    result.die1_ = ( rand() % 6 ) + 1;
    result.die2_ = ( rand() % 6 ) + 1;
    return result;
}

// Decisions after move________________________________________________________________________________________________

PlayerDecision GUIPlayer::decideBuyOrAuction( const PurchasableCard& card ) {
    if( playerPositions_ ) {
        parentWindow_->updatePlayerPositions( *playerPositions_ );
    }
    return parentWindow_->askBuyOrAuction( card.getName(), card.getPrice(), getCash() );
}

void GUIPlayer::informOfRandomCard( const RandomCard& card ) {
    parentWindow_->showRandomCard( card.describe() );
}

unsigned int GUIPlayer::decideBid( const PurchasableCard& card, unsigned int current_bid ) {
    return parentWindow_->askBid( card.getName(), current_bid, getCash() );
}

bool GUIPlayer::useGetOutOfJailCard() {
    return parentWindow_->askUseJailCard();
}

// Property operations_________________________________________________________________________________________________

void GUIPlayer::handleMortgageToggle( size_t card_index ) {
    if( card_index >= ownedCards_.size() ) {
        return;
    }

    if( auto* pc = dynamic_cast<PurchasableCard*>( ownedCards_[card_index] ) ) {
        if( pc->isMortgaged() ) {
            unmortgageCard( *pc );
        } else {
            mortgageCard( *pc );
        }
    }
}

void GUIPlayer::handleBuilding( size_t card_index ) {
    if( card_index >= ownedCards_.size() ) {
        return;
    }

    if( auto* prop = dynamic_cast<Property*>( ownedCards_[card_index] ) ) {
        build( *prop );
    }
}

// Trade operations____________________________________________________________________________________________________

bool GUIPlayer::wantsToTrade() {
    return parentWindow_->askWantsToTrade( name_ );
}

TradeOffer GUIPlayer::tradeOffer() {
    return parentWindow_->createTradeOffer( this, *players_ );
}

bool GUIPlayer::decideOnTrade( TradeOffer offer ) {
    return parentWindow_->askAcceptTrade( this, offer );
}
