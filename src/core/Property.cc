#include "Property.h"
#include <sstream>

using namespace std;

// Author: Jerzy Mossakowski
// Implements functionalities related to basic Property cards, including rent calculation, building, description

Property::Property( const string& name,
                    Color color,
                    unsigned int price,
                    unsigned int mortgage_val,
                    unsigned int house_cost,
                    const Rent& rents )
    : PurchasableCard( name, price, mortgage_val ), color_( color ), rentValues_( rents ), houseCost_( house_cost ) {}

unsigned int Property::calculateRent( const Game* game ) const {
    if( isMortgaged_ ) {
        return 0;
    }
    if( hotelCount_ > 0 ) {
        return rentValues_.rentWithHotel_;
    }
    switch( houseCount_ ) {
        case 1:
            return rentValues_.rentWith1House_;
        case 2:
            return rentValues_.rentWith2Houses_;
        case 3:
            return rentValues_.rentWith3Houses_;
        case 4:
            return rentValues_.rentWith4Houses_;
        default:
            return isMonopoly_ ? ( rentValues_.rentNoHouses_ * 2 ) : rentValues_.rentNoHouses_;
    }
}

void Property::buildHouse() {
    if( houseCount_ < 4 && hotelCount_ == 0 ) {
        ++houseCount_;
    }
}

void Property::buildHotel() {
    if( houseCount_ == 4 && hotelCount_ == 0 ) {
        houseCount_ = 0;
        hotelCount_ = 1;
    }
}

void Property::decreaseBuildingLevel() {
    if( hotelCount_ > 0 ) {
        hotelCount_ = 0;
        houseCount_ = 4;
    } else if( houseCount_ > 0 ) {
        --houseCount_;
    }
}

string Property::describe() const {
    ostringstream oss;

    oss << getName() << "\n\n";

    oss << "CZYNSZ $" << rentValues_.rentNoHouses_ << "\n";
    oss << "  Z jednym domkiem $" << rentValues_.rentWith1House_ << "\n";
    oss << "  Z dwoma domami $" << rentValues_.rentWith2Houses_ << "\n";
    oss << "  Z trzema domami $" << rentValues_.rentWith3Houses_ << "\n";
    oss << "  Z czterema domami $" << rentValues_.rentWith4Houses_ << "\n";
    oss << "  Z HOTELEM $" << rentValues_.rentWithHotel_ << "\n\n";

    oss << "Cena zakupu: $" << price_ << "\n";
    oss << "Wartość HIPOTECZNA: $" << mortgageValue_ << "\n";
    oss << "Ceny domów: $" << houseCost_ << " każdy\n";
    oss << "Ceny hoteli: $" << houseCost_ << " każdy plus 4 domy\n\n";

    oss << "Domy: " << houseCount_ << "   Hotele: " << hotelCount_;

    return oss.str();
}

Color Property::getColor() const {
    return color_;
}

void Property::setMonopoly( bool value ) {
    isMonopoly_ = value;
}

bool Property::isMonopoly() const {
    return isMonopoly_;
}

unsigned int Property::getHouseCost() const {
    return houseCost_;
}
unsigned int Property::getHouseCount() const {
    return houseCount_;
}
unsigned int Property::getHotelCount() const {
    return hotelCount_;
}
