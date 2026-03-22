#ifndef SPECIAL_PROPERTIES_H
#define SPECIAL_PROPERTIES_H

#include "Card.h"

// Author: Jerzy Mossakowski
// Implements purchasable cards other than standard properties, such as Railways and Utilities.

class Railway : public PurchasableCard {
public:
    Railway( const std::string& name );
    unsigned int calculateRent( const Game* game ) const override;
    std::string describe() const override;
};

class Utility : public PurchasableCard {
public:
    Utility( const std::string& name );
    unsigned int calculateRent( const Game* game ) const override;
    std::string describe() const override;
};

#endif  // SPECIAL_PROPERTIES_H
