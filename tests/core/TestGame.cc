#include <catch2/catch_test_macros.hpp>
#include "core/Game.h"
#include "core/Player.h"
#include "core/Property.h"
#include "core/SpecialProperties.h"

using namespace std;

class MockPlayer : public Player {
public:
    DiceResult nextRoll = { 1, 2 };
    PlayerDecision nextDecision = PlayerDecision::BUY;
    bool useJailCard = false;
    bool willTrade = false;
    TradeOffer nextOffer;
    bool acceptTrade = false;

    MockPlayer( string name ) : Player( move( name ) ) {}

    PlayerDecision decideBuyOrAuction( const PurchasableCard& card ) override { return nextDecision; }
    void informOfRandomCard( const RandomCard& card ) override {}
    unsigned int decideBid( const PurchasableCard& card, unsigned int currentBid ) override { return 0; }
    bool useGetOutOfJailCard() override { return useJailCard; }
    DiceResult takeTurn() override { return nextRoll; }
    DiceResult rollDice() const override { return nextRoll; }

    bool wantsToTrade() override { return willTrade; }
    TradeOffer tradeOffer() override { return nextOffer; }
    bool decideOnTrade( TradeOffer trade_offer ) override { return acceptTrade; }
};

Rent createGameTestRent() {
    return { 10, 50, 150, 450, 600, 1000 };
}

TEST_CASE( "Game Movement and Turn Rotation", "[Game]" ) {
    vector<unique_ptr<Player>> players;
    players.push_back( make_unique<MockPlayer>( "P1" ) );
    players.push_back( make_unique<MockPlayer>( "P2" ) );

    vector<unique_ptr<Space>> board;
    for( int i = 0; i < 10; ++i )
        board.push_back( make_unique<Space>( "S" + to_string( i ), SpaceType::GO ) );

    Game game( move( players ), move( board ), {}, {}, {} );
    MockPlayer* p1 = static_cast<MockPlayer*>( game.getPlayers()[0].get() );
    MockPlayer* p2 = static_cast<MockPlayer*>( game.getPlayers()[1].get() );

    SECTION( "Active player changes after turn" ) {
        game.runTurn();
        REQUIRE( game.getActivePlayer() == p2 );
        game.runTurn();
        REQUIRE( game.getActivePlayer() == p1 );
    }

    SECTION( "Passing Go adds cash" ) {
        p1->nextRoll = { 9, 2 };
        unsigned int initialCash = p1->getCash();
        game.runTurn();
        REQUIRE( p1->getCash() == initialCash + 200 );
        REQUIRE( game.getPlayerPositions().at( p1 ) == 1 );
    }
}

TEST_CASE( "Game Property Interaction", "[Game]" ) {
    vector<unique_ptr<Player>> players;
    players.push_back( make_unique<MockPlayer>( "P1" ) );
    players.push_back( make_unique<MockPlayer>( "P2" ) );

    Property* prop = new Property( "TestProp", Color::BROWN, 200, 100, 50, createGameTestRent() );
    Property* prop2 = new Property( "OtherProp", Color::BROWN, 200, 100, 50, createGameTestRent() );

    vector<unique_ptr<Space>> board;
    board.push_back( make_unique<Space>( "Go", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "PropSpace", SpaceType::PROPERTY, prop ) );
    board.push_back( make_unique<Space>( "OtherSpace", SpaceType::PROPERTY, prop2 ) );

    Game game( move( players ), move( board ), {}, {}, {} );
    MockPlayer* p1 = static_cast<MockPlayer*>( game.getPlayers()[0].get() );
    MockPlayer* p2 = static_cast<MockPlayer*>( game.getPlayers()[1].get() );

    SECTION( "Player buys unowned property" ) {
        p1->nextRoll = { 0, 1 };
        game.runTurn();

        REQUIRE( prop->getOwner() == p1 );
        REQUIRE( p1->getOwnedCards().size() == 1 );
        REQUIRE( p1->getCash() == 1300 );
    }

    SECTION( "Player pays base rent (No Monopoly)" ) {
        prop->setOwner( p2 );
        p2->addOwnedCard( prop );

        REQUIRE( prop->isMonopoly() == false );

        p1->nextRoll = { 0, 1 };
        game.runTurn();

        REQUIRE( p1->getCash() == 1490 );
        REQUIRE( p2->getCash() == 1510 );
    }

    SECTION( "Player pays double rent (Monopoly)" ) {
        prop->setOwner( p2 );
        prop2->setOwner( p2 );
        p2->addOwnedCard( prop );
        p2->addOwnedCard( prop2 );

        REQUIRE( prop->isMonopoly() == true );

        p1->nextRoll = { 0, 1 };
        game.runTurn();

        REQUIRE( p1->getCash() == 1480 );
        REQUIRE( p2->getCash() == 1520 );
    }
}

TEST_CASE( "Game Jail Logic", "[Game]" ) {
    vector<unique_ptr<Player>> players;
    players.push_back( make_unique<MockPlayer>( "P1" ) );

    vector<unique_ptr<Space>> board;
    board.push_back( make_unique<Space>( "Go", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "Jail", SpaceType::JAIL ) );
    board.push_back( make_unique<Space>( "Other", SpaceType::GO ) );

    Game game( move( players ), move( board ), {}, {}, {} );
    MockPlayer* p1 = static_cast<MockPlayer*>( game.getPlayers()[0].get() );

    SECTION( "Move to Jail via method" ) {
        game.movePlayerToJail( p1 );
        REQUIRE( game.getPlayerPositions().at( p1 ) == 1 );
        REQUIRE( p1->getTurnsInJail() == 3 );
    }

    SECTION( "Exit Jail by rolling double" ) {
        game.movePlayerToJail( p1 );
        p1->nextRoll = { 2, 2 };

        game.runTurn();

        REQUIRE( p1->getTurnsInJail() == 0 );
        REQUIRE( game.getPlayerPositions().at( p1 ) != 1 );
    }

    SECTION( "Stay in Jail if no double rolled" ) {
        game.movePlayerToJail( p1 );
        p1->nextRoll = { 1, 2 };

        game.runTurn();

        REQUIRE( p1->getTurnsInJail() == 2 );
        REQUIRE( game.getPlayerPositions().at( p1 ) == 1 );
    }
}

TEST_CASE( "Trading Logic", "[Game]" ) {
    vector<unique_ptr<Player>> players;
    players.push_back( make_unique<MockPlayer>( "P1" ) );
    players.push_back( make_unique<MockPlayer>( "P2" ) );

    Property* p1Card = new Property( "P1Prop", Color::BROWN, 100, 50, 50, createGameTestRent() );
    Property* p2Card = new Property( "P2Prop", Color::RED, 100, 50, 50, createGameTestRent() );

    vector<unique_ptr<Space>> board;
    board.push_back( make_unique<Space>( "Go", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "S1", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "S2", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "S3", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "S4", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "P1Space", SpaceType::PROPERTY, p1Card ) );
    board.push_back( make_unique<Space>( "P2Space", SpaceType::PROPERTY, p2Card ) );

    Game game( move( players ), move( board ), {}, {}, {} );
    MockPlayer* p1 = static_cast<MockPlayer*>( game.getPlayers()[0].get() );
    MockPlayer* p2 = static_cast<MockPlayer*>( game.getPlayers()[1].get() );

    p1->addOwnedCard( p1Card );
    p1Card->setOwner( p1 );
    p2->addOwnedCard( p2Card );
    p2Card->setOwner( p2 );

    SECTION( "Successful Trade" ) {
        p1->willTrade = true;
        p2->acceptTrade = true;
        p1->nextRoll = { 1, 1 };

        TradeOffer offer;
        offer.receiver_ = p2;
        offer.cardOffered_ = p1Card;
        offer.cardRequested_ = p2Card;
        offer.cashOffered_ = 100;
        p1->nextOffer = offer;

        game.runTurn();

        CHECK( p1Card->getOwner() == p2 );
        CHECK( p2Card->getOwner() == p1 );
        CHECK( p1->getCash() == 1400 );
        CHECK( p2->getCash() == 1600 );
    }

    SECTION( "Rejected Trade" ) {
        p1->willTrade = true;
        p2->acceptTrade = false;
        p1->nextRoll = { 1, 1 };

        TradeOffer offer;
        offer.receiver_ = p2;
        offer.cardOffered_ = p1Card;
        p1->nextOffer = offer;

        game.runTurn();

        CHECK( p1Card->getOwner() == p1 );
        CHECK( p1->getCash() == 1500 );
    }
}

TEST_CASE( "Forced Liquidation", "[Player]" ) {
    vector<unique_ptr<Space>> board;
    board.push_back( make_unique<Space>( "Go", SpaceType::GO ) );

    auto p1 = make_unique<MockPlayer>( "P1" );
    Property* prop = new Property( "HighRent", Color::DARKBLUE, 400, 200, 200, createGameTestRent() );
    p1->addOwnedCard( prop );
    prop->setOwner( p1.get() );

    vector<unique_ptr<Player>> players;
    players.push_back( move( p1 ) );
    Game game( move( players ), move( board ), {}, {}, {} );
    MockPlayer* player = static_cast<MockPlayer*>( game.getPlayers()[0].get() );

    SECTION( "Forced Mortgage" ) {
        player->subtractCash( 1500 );
        player->subtractCash( 100 );

        CHECK( prop->isMortgaged() == true );
        CHECK( player->getCash() == 100 );
    }

    SECTION( "Player Bankruptcy" ) {
        player->subtractCash( 1500 );
        player->mortgageCard( *prop );
        player->subtractCash( 500 );

        CHECK( player->isLost() == true );
        CHECK( player->getCash() == 0 );
    }
}

TEST_CASE( "Special Properties Rent", "[Cards]" ) {
    auto p1 = make_unique<MockPlayer>( "P1" );
    auto p2 = make_unique<MockPlayer>( "P2" );

    vector<unique_ptr<Space>> board;
    board.push_back( make_unique<Space>( "S", SpaceType::GO ) );
    board.push_back( make_unique<Space>( "S1", SpaceType::GO ) );

    vector<unique_ptr<Player>> players;
    players.push_back( move( p1 ) );
    players.push_back( move( p2 ) );

    Game game( move( players ), move( board ), {}, {}, {} );
    MockPlayer* owner = static_cast<MockPlayer*>( game.getPlayers()[0].get() );

    SECTION( "Railways" ) {
        Railway* r1 = new Railway( "R1" );
        Railway* r2 = new Railway( "R2" );
        owner->addOwnedCard( r1 );
        r1->setOwner( owner );

        CHECK( r1->calculateRent( &game ) == 25 );

        owner->addOwnedCard( r2 );
        r2->setOwner( owner );
        CHECK( r1->calculateRent( &game ) == 50 );
    }

    SECTION( "Utilities" ) {
        Utility* u1 = new Utility( "U1" );
        owner->addOwnedCard( u1 );
        u1->setOwner( owner );

        owner->nextRoll = { 1, 2 };
        game.runTurn();

        CHECK( u1->calculateRent( &game ) == 3 * 4 );

        Utility* u2 = new Utility( "U2" );
        owner->addOwnedCard( u2 );
        u2->setOwner( owner );

        CHECK( u1->calculateRent( &game ) == 3 * 10 );
    }
}

TEST_CASE( "Building Rules", "[Player]" ) {
    vector<unique_ptr<Space>> board;
    Property* p1 = new Property( "P1", Color::GREEN, 300, 150, 200, createGameTestRent() );
    Property* p2 = new Property( "P2", Color::GREEN, 300, 150, 200, createGameTestRent() );

    board.push_back( make_unique<Space>( "S1", SpaceType::PROPERTY, p1 ) );
    board.push_back( make_unique<Space>( "S2", SpaceType::PROPERTY, p2 ) );

    vector<unique_ptr<Player>> players;
    players.push_back( make_unique<MockPlayer>( "P1" ) );
    Game game( move( players ), move( board ), {}, {}, {} );

    Player* player = game.getPlayers()[0].get();
    player->addOwnedCard( p1 );
    p1->setOwner( player );
    player->addOwnedCard( p2 );
    p2->setOwner( player );

    SECTION( "Monopoly Check" ) {
        p1->setMonopoly( false );
        player->build( *p1 );
        CHECK( p1->getHouseCount() == 0 );
    }

    SECTION( "Even Building" ) {
        p1->setMonopoly( true );
        p2->setMonopoly( true );

        player->build( *p1 );
        CHECK( p1->getHouseCount() == 1 );

        player->build( *p1 );
        CHECK( p1->getHouseCount() == 1 );

        player->build( *p2 );
        CHECK( p2->getHouseCount() == 1 );
    }
}
