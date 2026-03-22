#ifndef PROPERTY_H
#define PROPERTY_H

#include <cstdint>
#include <string>
#include "Card.h"

// Author: Jerzy Mossakowski
// Implements property represented as a card that can be owned by the player.

enum class Color : std::uint8_t { BROWN, LIGHTBLUE, PINK, ORANGE, RED, YELLOW, GREEN, DARKBLUE };

struct Rent {
    unsigned int rentNoHouses_;
    unsigned int rentWith1House_;
    unsigned int rentWith2Houses_;
    unsigned int rentWith3Houses_;
    unsigned int rentWith4Houses_;
    unsigned int rentWithHotel_;
};

class Property : public PurchasableCard {
public:
    Property( const std::string& name,
              Color color,
              unsigned int price,
              unsigned int mortgage_val,
              unsigned int house_cost,
              const Rent& rents );

    unsigned int calculateRent( const Game* game ) const override;

    void buildHouse();
    void buildHotel();
    void decreaseBuildingLevel();

    void setMonopoly( bool value );
    bool isMonopoly() const;
    unsigned int getHouseCost() const;
    unsigned int getHouseCount() const;
    unsigned int getHotelCount() const;
    Color getColor() const;

    std::string describe() const override;

private:
    const Color color_;
    const Rent rentValues_;
    const unsigned int houseCost_;

    unsigned int houseCount_ = 0;
    unsigned int hotelCount_ = 0;
    bool isMonopoly_ = false;
};

#endif  // PROPERTY_H
