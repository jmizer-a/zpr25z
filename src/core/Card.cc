#include "Card.h"

using namespace std;

// Author: Jerzy Mossakowski
// Implements abstract Card classes: Card, PurchasableCard, RandomCard and: getters, setters, and effect application

Card::Card( const string& name ) : name_( name ) {}

PurchasableCard::PurchasableCard( const string& name, unsigned int price, unsigned int mortgage_value )
    : Card( name ), price_( price ), mortgageValue_( mortgage_value ) {}

Player* PurchasableCard::getOwner() const {
    return owner_;
}

void PurchasableCard::setOwner( Player* player ) {
    owner_ = player;
}

unsigned int PurchasableCard::getPrice() const {
    return price_;
}
unsigned int PurchasableCard::getMortgageValue() const {
    return mortgageValue_;
}

bool PurchasableCard::isMortgaged() const {
    return isMortgaged_;
}
void PurchasableCard::setMortgaged( bool state ) {
    isMortgaged_ = state;
}

string RandomCard::describe() const {
    return name_ + "\n" + description_;
}

void RandomCard::apply( Game* game, Player* player ) const {
    if( effect_ )
        effect_( game, player );
}
