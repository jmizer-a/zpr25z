#include <catch2/catch_test_macros.hpp>
#include "core/Property.h"

using namespace std;

Rent createRent() {
    return { 10, 50, 150, 450, 600, 1000 };
}

TEST_CASE( "Initial rent is base value", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    REQUIRE( p.calculateRent( nullptr ) == 10 );
}

TEST_CASE( "Monopoly doubles base rent", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    p.setMonopoly( true );
    REQUIRE( p.calculateRent( nullptr ) == 20 );
}

TEST_CASE( "Mortgage rent is zero", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    p.setMortgaged( true );
    REQUIRE( p.calculateRent( nullptr ) == 0 );
}

TEST_CASE( "Unmortgage restores rent", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    p.setMortgaged( true );
    p.setMortgaged( false );
    REQUIRE( p.calculateRent( nullptr ) == 10 );
}

TEST_CASE( "Rent with one house", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    p.buildHouse();
    REQUIRE( p.calculateRent( nullptr ) == 50 );
}

TEST_CASE( "Monopoly does not double rent with houses", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    p.setMonopoly( true );
    p.buildHouse();
    REQUIRE( p.calculateRent( nullptr ) == 50 );
}

TEST_CASE( "Rent with hotel", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    for( int i = 0; i < 4; ++i )
        p.buildHouse();
    p.buildHotel();
    REQUIRE( p.calculateRent( nullptr ) == 1000 );
}

TEST_CASE( "House limit is four", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    for( int i = 0; i < 10; ++i )
        p.buildHouse();
    REQUIRE( p.calculateRent( nullptr ) == 600 );
}

TEST_CASE( "Hotel requirements not met", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    p.buildHouse();
    p.buildHotel();
    REQUIRE( p.calculateRent( nullptr ) == 50 );
}

TEST_CASE( "Get mortgage and price values", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    REQUIRE( p.getPrice() == 200 );
    REQUIRE( p.getMortgageValue() == 100 );
}

TEST_CASE( "Describe contains required info", "[Property]" ) {
    Property p( "Ulica", Color::LIGHTBLUE, 200, 100, 50, createRent() );
    string d = p.describe();
    REQUIRE( d.find( "Ulica" ) != string::npos );
    REQUIRE( d.find( "CZYNSZ $10" ) != string::npos );
    REQUIRE( d.find( "200" ) != string::npos );
    REQUIRE( d.find( "100" ) != string::npos );
}
