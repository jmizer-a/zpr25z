#ifndef SPACE_H
#define SPACE_H

#include <cstdint>
#include <string>
#include "Card.h"

enum class SpaceType : std::uint8_t {
    PROPERTY,
    RAILWAY,
    UTILITY,
    CHANCE,
    COMMUNITY_CHEST,
    TAX,
    GO_TO_JAIL,
    FREE_PARKING,
    JAIL,
    GO
};

// Author: Jerzy Mossakowski
// Implements individual spaces on the game board

class Space {
public:
    Space( std::string name, SpaceType type, Card* card = nullptr )
        : name_( std::move( name ) ), type_( type ), associatedCard_( card ) {}
    std::string getName() const { return name_; }
    SpaceType getType() const { return type_; }
    Card* getAssociatedCard() const { return associatedCard_; }

private:
    std::string name_;
    SpaceType type_;
    Card* associatedCard_;
};

#endif  // SPACE_H
