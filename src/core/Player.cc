#include "Player.h"
#include <iostream>

using namespace std;

// Author: Jerzy Mossakowski
// Implements player-related functionalities, including cash management, property handling, building operations within
// abstract Player class, providing a foundation for different player types such as GUI Player and AI Player

// Getters_____________________________________________________________________________________________________________

const string& Player::getName() const {
    return name_;
}
unsigned int Player::getCash() const {
    return cash_;
}
const vector<Card*>& Player::getOwnedCards() const {
    return ownedCards_;
}
unsigned int Player::getTurnsInJail() const {
    return turnsInJail_;
}

bool Player::isLost() const {
    return lost_;
}

// Falling below zero__________________________________________________________________________________________________

void Player::subtractCash( const unsigned int amount ) {
    if( amount <= cash_ ) {
        cash_ -= amount;
        return;
    }
    forcedMortgage( amount );
    forcedSell( amount );
    if( cash_ >= amount ) {
        cash_ -= amount;
    } else {
        cash_ = 0;
        lose();
    }
}

void Player::forcedMortgage( const unsigned int amount ) {
    if( cash_ < amount ) {
        for( Card* card : ownedCards_ ) {
            PurchasableCard* pc = dynamic_cast<PurchasableCard*>( card );
            if( pc && !pc->isMortgaged() ) {
                Property* prop = dynamic_cast<Property*>( pc );
                bool hasBuildings = prop && ( prop->getHouseCount() > 0 || prop->getHotelCount() > 0 );
                if( !hasBuildings ) {
                    mortgageCard( *pc );
                    if( cash_ >= amount )
                        break;
                }
            }
        }
    }
}

void Player::forcedSell( const unsigned int amount ) {
    while( cash_ < amount ) {
        Property* bestProperty = nullptr;
        int maxLevel = 0;
        for( Card* card : ownedCards_ ) {
            Property* p = dynamic_cast<Property*>( card );
            if( p && canSellBuilding( p ) ) {
                int level = p->getHotelCount() > 0 ? 5 : p->getHouseCount();
                if( level > maxLevel ) {
                    maxLevel = level;
                    bestProperty = p;
                }
            }
        }
        if( !bestProperty )
            break;
        addCash( bestProperty->getHouseCost() / 2 );
        bestProperty->decreaseBuildingLevel();
    }
}

bool Player::canSellBuilding( Property* property_card ) const {
    if( !board_ || !property_card )
        return false;
    int currentLevel = property_card->getHotelCount() > 0 ? 5 : property_card->getHouseCount();
    if( currentLevel == 0 )
        return false;
    for( const auto& space : *board_ ) {
        Property* other = dynamic_cast<Property*>( space->getAssociatedCard() );
        if( other && other->getColor() == property_card->getColor() ) {
            int otherLevel = other->getHotelCount() > 0 ? 5 : other->getHouseCount();
            if( otherLevel > currentLevel )
                return false;
        }
    }
    return true;
}

// Building operations_________________________________________________________________________________________________

void Player::build( Property& property_card ) {
    vector<Property*> group = findColorGroup( property_card );
    if( !property_card.isMonopoly() || property_card.isMortgaged() )
        return;
    int currentLevel = property_card.getHotelCount() > 0 ? 5 : property_card.getHouseCount();
    if( isOtherPropertyUndeveloped( group, currentLevel ) )
        return;
    if( cash_ >= property_card.getHouseCost() ) {
        subtractCash( property_card.getHouseCost() );
        if( property_card.getHouseCount() == 4 )
            property_card.buildHotel();
        else
            property_card.buildHouse();
    }
}

vector<Property*> Player::findColorGroup( Property& property_card ) {
    vector<Property*> group;
    if( !board_ )
        return group;
    for( const auto& space : *board_ ) {
        Property* p = dynamic_cast<Property*>( space->getAssociatedCard() );
        if( p && p->getColor() == property_card.getColor() ) {
            group.push_back( p );
        }
    }
    return group;
}

int Player::countCardsInColorGroup( Property& property_card ) {
    int ownedInColor = 0;
    for( Card* card : ownedCards_ ) {
        Property* p = dynamic_cast<Property*>( card );
        if( p && p->getColor() == property_card.getColor() )
            ++ownedInColor;
    }
    return ownedInColor;
}

bool Player::isOtherPropertyUndeveloped( vector<Property*> group, const int level ) {
    for( Property* p : group ) {
        if( p->isMortgaged() )
            return true;
        int otherLevel = p->getHotelCount() > 0 ? 5 : p->getHouseCount();
        if( level > otherLevel )
            return true;
    }
    return false;
}

// Property operations_________________________________________________________________________________________________

void Player::mortgageCard( Card& card ) {
    PurchasableCard* pCard = dynamic_cast<PurchasableCard*>( &card );
    if( pCard && !pCard->isMortgaged() ) {
        Property* property = dynamic_cast<Property*>( pCard );
        if( property && ( property->getHouseCount() > 0 || property->getHotelCount() > 0 ) ) {
            return;
        }
        pCard->setMortgaged( true );
        addCash( pCard->getMortgageValue() );
    }
}

void Player::unmortgageCard( Card& card ) {
    PurchasableCard* pCard = dynamic_cast<PurchasableCard*>( &card );
    if( pCard && pCard->isMortgaged() ) {
        unsigned int mortgageVal = pCard->getMortgageValue();
        unsigned int fee = mortgageVal / 10;
        unsigned int cost = mortgageVal + fee;
        if( cash_ >= cost ) {
            subtractCash( cost );
            pCard->setMortgaged( false );
        }
    }
}

void Player::addOwnedCard( Card* card ) {
    ownedCards_.push_back( card );
    Property* pCard = dynamic_cast<Property*>( card );
    if( pCard ) {
        vector<Property*> group = findColorGroup( *pCard );
        int ownedInColor = countCardsInColorGroup( *pCard );
        bool hasMonopoly = ( group.size() == ownedInColor );

        for( Property* p : group ) {
            p->setMonopoly( hasMonopoly );
        }
    }
}

void Player::removeOwnedCard( const string& name ) {
    for( auto it = ownedCards_.begin(); it != ownedCards_.end(); ++it ) {
        if( ( *it )->getName() == name ) {
            ownedCards_.erase( it );
            return;
        }
    }
}

bool Player::isMonopoly( Property& property_card ) {
    vector<Property*> group = findColorGroup( property_card );
    int propertiesInColor = group.size();
    int ownedInColor = countCardsInColorGroup( property_card );
    return propertiesInColor == ownedInColor;
}

// Cash operations_____________________________________________________________________________________________________

void Player::addCash( const unsigned int amount ) {
    cash_ += amount;
}

void Player::payRepairCosts( const unsigned int cost_per_house, const unsigned int cost_per_hotel ) {
    unsigned int totalCost = 0;
    for( Card* card : ownedCards_ ) {
        Property* property = dynamic_cast<Property*>( card );
        if( property ) {
            totalCost += property->getHouseCount() * cost_per_house;
            totalCost += property->getHotelCount() * cost_per_hotel;
        }
    }
    subtractCash( totalCost );
}

// Other operations____________________________________________________________________________________________________

void Player::addJailFreeCard() {
    ownedCards_.push_back(
        new RandomCard( "Ucieczka z więzienia", "Zagraj tę kartę, aby uciec z więzienia bez kaucji" ) );
}

void Player::setTurnsInJail( const unsigned int turns ) {
    turnsInJail_ = turns;
}

void Player::lose() {
    lost_ = true;
}
