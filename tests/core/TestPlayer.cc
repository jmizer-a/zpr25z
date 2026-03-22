#include <catch2/catch_test_macros.hpp>
#include "core/Player.h"
#include "core/Property.h"
#include "core/Space.h"

using namespace std;

class TestPlayer : public Player {
public:
    TestPlayer( string name ) : Player( move( name ) ) {}
    PlayerDecision decideBuyOrAuction( const PurchasableCard& card ) override { return PlayerDecision::BUY; }
    void informOfRandomCard( const RandomCard& card ) override {}
    unsigned int decideBid( const PurchasableCard& card, unsigned int currentBid ) override { return 0; }
    bool useGetOutOfJailCard() override { return false; }
    DiceResult takeTurn() override { return { 1, 1 }; }
    DiceResult rollDice() const override { return { 1, 1 }; }
    bool wantsToTrade() override { return false; }
    TradeOffer tradeOffer() override { return {}; }
    bool decideOnTrade( TradeOffer trade_offer ) override { return false; }
};

Rent createTestRent() {
    return { 10, 50, 150, 450, 600, 1000 };
}

TEST_CASE( "Player Monopoly and Building Logic", "[Player]" ) {
    TestPlayer player( "Tester" );

    auto p1 = make_unique<Property>( "Ulica 1", Color::BROWN, 100, 50, 50, createTestRent() );
    auto p2 = make_unique<Property>( "Ulica 2", Color::BROWN, 100, 50, 50, createTestRent() );

    Property* p1_ptr = p1.get();
    Property* p2_ptr = p2.get();

    vector<unique_ptr<Space>> board;
    board.push_back( make_unique<Space>( "S1", SpaceType::PROPERTY, p1_ptr ) );
    board.push_back( make_unique<Space>( "S2", SpaceType::PROPERTY, p2_ptr ) );

    vector<unique_ptr<Player>> players;
    unordered_map<Player*, unsigned int> positions;
    player.setGameContext( players, board, positions );

    SECTION( "Monopoly Requirement" ) {
        player.addOwnedCard( p1_ptr );

        player.build( *p1_ptr );
        REQUIRE( p1_ptr->getHouseCount() == 0 );
        REQUIRE( p1_ptr->isMonopoly() == false );

        player.addOwnedCard( p2_ptr );
        REQUIRE( p1_ptr->isMonopoly() == true );
        REQUIRE( p2_ptr->isMonopoly() == true );
    }

    SECTION( "Even Building and Hotel Rules" ) {
        player.addOwnedCard( p1_ptr );
        player.addOwnedCard( p2_ptr );

        player.build( *p1_ptr );
        REQUIRE( p1_ptr->getHouseCount() == 1 );

        player.build( *p1_ptr );
        REQUIRE( p1_ptr->getHouseCount() == 1 );

        player.build( *p2_ptr );
        REQUIRE( p2_ptr->getHouseCount() == 1 );

        for( int i = 0; i < 3; ++i ) {
            player.build( *p1_ptr );
            player.build( *p2_ptr );
        }

        REQUIRE( p1_ptr->getHouseCount() == 4 );
        REQUIRE( p2_ptr->getHouseCount() == 4 );

        player.build( *p1_ptr );
        REQUIRE( p1_ptr->getHouseCount() == 0 );
        REQUIRE( p1_ptr->getHotelCount() == 1 );

        REQUIRE( p2_ptr->getHouseCount() == 4 );
    }
}

TEST_CASE( "Player Mortgage and Repair Logic", "[Player]" ) {
    TestPlayer player( "Tester" );

    vector<unique_ptr<Space>> board;
    vector<unique_ptr<Player>> players;
    unordered_map<Player*, unsigned int> positions;

    SECTION( "Mortgage Restrictions" ) {
        Property p( "Ulica", Color::BROWN, 100, 50, 50, createTestRent() );

        board.push_back( make_unique<Space>( "S1", SpaceType::PROPERTY, &p ) );
        player.setGameContext( players, board, positions );

        player.addOwnedCard( &p );
        p.buildHouse();

        player.mortgageCard( p );
        REQUIRE( p.isMortgaged() == false );
        REQUIRE( player.getCash() == 1500 );
    }

    SECTION( "Unmortgage Fee" ) {
        Property p( "Ulica", Color::BROWN, 100, 50, 50, createTestRent() );
        board.push_back( make_unique<Space>( "S1", SpaceType::PROPERTY, &p ) );
        player.setGameContext( players, board, positions );

        player.addOwnedCard( &p );

        player.mortgageCard( p );
        REQUIRE( player.getCash() == 1550 );

        player.unmortgageCard( p );
        REQUIRE( p.isMortgaged() == false );
        REQUIRE( player.getCash() == 1495 );
    }

    SECTION( "Repair Costs" ) {
        Property p1( "P1", Color::BROWN, 100, 50, 50, createTestRent() );
        Property p2( "P2", Color::RED, 200, 100, 100, createTestRent() );

        board.push_back( make_unique<Space>( "S1", SpaceType::PROPERTY, &p1 ) );
        board.push_back( make_unique<Space>( "S2", SpaceType::PROPERTY, &p2 ) );
        player.setGameContext( players, board, positions );

        p1.buildHouse();
        p1.buildHouse();

        for( int i = 0; i < 4; ++i )
            p2.buildHouse();
        p2.buildHotel();

        player.addOwnedCard( &p1 );
        player.addOwnedCard( &p2 );

        player.payRepairCosts( 25, 100 );
        REQUIRE( player.getCash() == 1350 );
    }
}

TEST_CASE( "Player Liquidation and Bankruptcy", "[Player]" ) {
    TestPlayer player( "Tester" );
    vector<unique_ptr<Space>> board;
    vector<unique_ptr<Player>> players;
    unordered_map<Player*, unsigned int> positions;

    Property* p1 = new Property( "P1", Color::BROWN, 100, 50, 50, createTestRent() );
    board.push_back( make_unique<Space>( "S1", SpaceType::PROPERTY, p1 ) );
    player.setGameContext( players, board, positions );
    player.addOwnedCard( p1 );
    p1->setOwner( &player );

    SECTION( "Forced House Sale" ) {
        p1->setMonopoly( true );
        player.build( *p1 );
        REQUIRE( p1->getHouseCount() == 1 );

        player.subtractCash( 1460 );
        player.subtractCash( 50 );

        REQUIRE( p1->getHouseCount() == 0 );
        REQUIRE( player.getCash() == 15 );
    }

    SECTION( "Bankruptcy" ) {
        player.subtractCash( 2000 );
        REQUIRE( player.isLost() == true );
        REQUIRE( player.getCash() == 0 );
    }
}
