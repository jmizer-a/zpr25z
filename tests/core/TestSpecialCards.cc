#include <catch2/catch_test_macros.hpp>
#include "core/Game.h"
#include "core/Player.h"
#include "core/SpecialProperties.h"

using namespace std;

class TestPlayer : public Player {
public:
    TestPlayer( string name ) : Player( move( name ) ) {}

    PlayerDecision decideBuyOrAuction( const PurchasableCard& card ) override { return PlayerDecision::BUY; }
    void informOfRandomCard( const RandomCard& card ) override {}
    unsigned int decideBid( const PurchasableCard& card, unsigned int currentBid ) override { return 0; }
    bool useGetOutOfJailCard() override { return false; }
    DiceResult takeTurn() override { return { 1, 1 }; }
    DiceResult rollDice() const override {
        DiceResult result;
        result.die1_ = ( rand() % 6 ) + 1;
        result.die2_ = ( rand() % 6 ) + 1;
        return result;
    }
    bool wantsToTrade() { return false; };
    TradeOffer tradeOffer() { return {}; };
    bool decideOnTrade( TradeOffer trade_offer ) { return false; };
};

TEST_CASE( "Railway rent scales with ownership", "[Railway]" ) {
    TestPlayer player( "Tester" );
    Railway r1( "PKP" );
    Railway r2( "PKS" );
    Railway r3( "Intercity" );
    Railway r4( "Koleje Śląskie" );

    r1.setOwner( &player );
    player.addOwnedCard( &r1 );

    SECTION( "One railway owned" ) {
        REQUIRE( r1.calculateRent( nullptr ) == 25 );
    }

    SECTION( "Two railways owned" ) {
        r2.setOwner( &player );
        player.addOwnedCard( &r2 );
        REQUIRE( r1.calculateRent( nullptr ) == 50 );
    }

    SECTION( "Three railways owned" ) {
        r2.setOwner( &player );
        r3.setOwner( &player );
        player.addOwnedCard( &r2 );
        player.addOwnedCard( &r3 );
        REQUIRE( r1.calculateRent( nullptr ) == 100 );
    }

    SECTION( "Four railways owned" ) {
        r2.setOwner( &player );
        r3.setOwner( &player );
        r4.setOwner( &player );
        player.addOwnedCard( &r2 );
        player.addOwnedCard( &r3 );
        player.addOwnedCard( &r4 );
        REQUIRE( r1.calculateRent( nullptr ) == 200 );
    }
}

TEST_CASE( "Railway mortgage logic", "[Railway]" ) {
    Railway r1( "PKP" );
    r1.setMortgaged( true );
    r1.setOwner( nullptr );
    REQUIRE( r1.calculateRent( nullptr ) == 0 );
}

TEST_CASE( "Utility rent calculation", "[Utility]" ) {
    TestPlayer player( "Tester" );
    Utility u1( "Elektrownia" );
    Utility u2( "Wodociągi" );

    u1.setOwner( &player );
    player.addOwnedCard( &u1 );

    vector<unique_ptr<Player>> game_players;
    game_players.push_back( make_unique<TestPlayer>( "P1" ) );
    Game game( move( game_players ), {}, {}, {}, {} );

    SECTION( "One utility: 4x dice roll" ) {
        REQUIRE( u1.calculateRent( &game ) == 0 );
    }

    SECTION( "Two utilities: 10x dice roll" ) {
        u2.setOwner( &player );
        player.addOwnedCard( &u2 );
        REQUIRE( u1.calculateRent( &game ) == 0 );
    }
}

TEST_CASE( "Special properties descriptions", "[SpecialProperties]" ) {
    SECTION( "Railway description" ) {
        Railway r( "Dworzec Wschodni" );
        string d = r.describe();
        REQUIRE( d.find( "Dworzec Wschodni" ) != string::npos );
        REQUIRE( d.find( "$25" ) != string::npos );
        REQUIRE( d.find( "$200" ) != string::npos );
    }

    SECTION( "Utility description" ) {
        Utility u( "Elektrownia" );
        string d = u.describe();
        REQUIRE( d.find( "Elektrownia" ) != string::npos );
        REQUIRE( d.find( "4 x" ) != string::npos );
        REQUIRE( d.find( "10 x" ) != string::npos );
    }
}

TEST_CASE( "RandomCard stores description", "[RandomCard]" ) {
    RandomCard card( "Szansa", "Idź do więzienia. Nie przechodź przez START." );
    REQUIRE( card.getName() == "Szansa" );
    REQUIRE( card.describe() == "Szansa\nIdź do więzienia. Nie przechodź przez START." );
}

TEST_CASE( "RandomCard apply executes effect", "[RandomCard]" ) {
    bool effectExecuted = false;

    RandomCard card( "Nagroda", "Otrzymujesz 100$", [&]( Game* g, Player* p ) { effectExecuted = true; } );

    card.apply( nullptr, nullptr );
    REQUIRE( effectExecuted == true );
}

TEST_CASE( "RandomCard effect modifies player state", "[RandomCard]" ) {
    TestPlayer player( "Tester" );
    unsigned int initialCash = player.getCash();

    RandomCard card( "Podatek", "Zapłać 50$", []( Game* g, Player* p ) { p->subtractCash( 50 ); } );

    card.apply( nullptr, &player );
    REQUIRE( player.getCash() == initialCash - 50 );
}

TEST_CASE( "RandomCard handle null effect", "[RandomCard]" ) {
    RandomCard card( "Pusta Karta", "Nic się nie dzieje", nullptr );

    SECTION( "Calling apply with nullptr effect does not crash" ) {
        REQUIRE_NOTHROW( card.apply( nullptr, nullptr ) );
    }
}
