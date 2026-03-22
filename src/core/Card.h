#ifndef CARD_H
#define CARD_H

#include <functional>
#include <string>
#include <vector>

class Player;
class Game;

// Author: Jerzy Mossakowski
// Implements basic card that can be owned by the player.
// Inheriting classes implement specific card types such as Property, Chance, Railroad etc.

class Card {
public:
    virtual ~Card() = default;
    virtual std::string getName() const { return name_; }
    virtual std::string describe() const { return getName(); }

protected:
    Card( const std::string& name );
    const std::string name_;
};

class PurchasableCard : public Card {
public:
    PurchasableCard( const std::string& name, unsigned int price, unsigned int mortgage_value );

    Player* getOwner() const;
    void setOwner( Player* player );

    unsigned int getPrice() const;
    unsigned int getMortgageValue() const;

    bool isMortgaged() const;
    void setMortgaged( bool state );

    virtual unsigned int calculateRent( const Game* game ) const = 0;

protected:
    Player* owner_ = nullptr;
    const unsigned int price_;
    const unsigned int mortgageValue_;
    bool isMortgaged_ = false;
};

class RandomCard : public Card {
public:
    RandomCard( const std::string& name,
                const std::string& description,
                std::function<void( Game*, Player* )> effect = nullptr )
        : Card( name ), description_( description ), effect_( effect ) {}

    std::string describe() const override;
    void apply( Game* game, Player* player ) const;

private:
    const std::string description_;
    const std::function<void( Game*, Player* )> effect_;
};

#endif  // CARD_H
