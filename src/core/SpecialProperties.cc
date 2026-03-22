#include "SpecialProperties.h"
#include <sstream>
#include "core/Game.h"
#include "core/Player.h"

using namespace std;

// Author: Jerzy Mossakowski
// Implements functionalities related to special property cards: Railway and Utility, including rent calculation
// and description

Railway::Railway( const string& name ) : PurchasableCard( name, 200, 100 ) {}

unsigned int Railway::calculateRent( const Game* game ) const {
    if( !owner_ || isMortgaged_ ) {
        return 0;
    }

    unsigned int count = 0;
    for( Card* card : owner_->getOwnedCards() ) {
        if( dynamic_cast<Railway*>( card ) ) {
            count++;
        }
    }

    switch( count ) {
        case 1:
            return 25;
        case 2:
            return 50;
        case 3:
            return 100;
        case 4:
            return 200;
        default:
            return 0;
    }
}

string Railway::describe() const {
    ostringstream oss;

    oss << getName() << "\n\n";

    oss << "CZYNSZ $25\n";
    oss << "  Jeśli 2 koleje są w posiadaniu gracza $50\n";
    oss << "  Jeśli 3 koleje są w posiadaniu gracza $100\n";
    oss << "  Jeśli 4 koleje są w posiadaniu gracza $200\n\n";

    oss << "Cena zakupu: $200\n";
    oss << "Wartość HIPOTECZNA: $100\n";

    return oss.str();
}

Utility::Utility( const string& name ) : PurchasableCard( name, 150, 75 ) {}

unsigned int Utility::calculateRent( const Game* game ) const {
    if( !owner_ || isMortgaged_ || !game ) {
        return 0;
    }

    unsigned int count = 0;
    for( Card* card : owner_->getOwnedCards() ) {
        if( dynamic_cast<Utility*>( card ) ) {
            count++;
        }
    }

    unsigned int roll = game->getLastDiceRoll();
    return ( count >= 2 ) ? ( roll * 10 ) : ( roll * 4 );
}

string Utility::describe() const {
    ostringstream oss;

    oss << getName() << "\n\n";

    oss << "Jeśli 1 zakład jest w posiadaniu gracza, renta = 4 x wynik na kościach\n\n";

    oss << "Jeśli oba zakłady są w posiadaniu gracza, renta = 10 x wynik na kościach\n";

    return oss.str();
}
