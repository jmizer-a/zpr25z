#include "GameConfig.h"
#include "gui/GUIPlayer.h"
#include "Player.h"
#include "Property.h"
#include "SpecialProperties.h"

using namespace std;

// Author: Jerzy Mossakowski
// Serves as a script to configure and initialize the Monopoly game with all necessary components

// Helper functions____________________________________________________________________________________________________

#define LAMBDA []( Game * g, Player * p )

void prop( vector<unique_ptr<Card>>& d, string n, Color c, int p, int m, int h, Rent r ) {
    d.push_back( make_unique<Property>( n, c, p, m, h, r ) );
}

void rail( vector<unique_ptr<Card>>& d, string n ) {
    d.push_back( make_unique<Railway>( n ) );
}

void util( vector<unique_ptr<Card>>& d, string n ) {
    d.push_back( make_unique<Utility>( n ) );
}

void spac( vector<unique_ptr<Space>>& b, string n, SpaceType t, Card* c = nullptr ) {
    b.push_back( make_unique<Space>( n, t, c ) );
}

void chan( vector<unique_ptr<RandomCard>>& d, string desc, function<void( Game*, Player* )> f ) {
    d.push_back( make_unique<RandomCard>( "Szansa", desc, f ) );
}

void ches( vector<unique_ptr<RandomCard>>& d, string desc, function<void( Game*, Player* )> f ) {
    d.push_back( make_unique<RandomCard>( "Kasa społeczna", desc, f ) );
}

// Initial object generators___________________________________________________________________________________________

vector<unique_ptr<Card>> getInitialDeck() {
    vector<unique_ptr<Card>> d;
    prop( d, "Konopacka", Color::BROWN, 60, 30, 50, Rent{ 2, 10, 30, 90, 160, 250 } );
    prop( d, "Stalowa", Color::BROWN, 60, 30, 50, Rent{ 4, 20, 60, 180, 320, 450 } );
    rail( d, "Dworzec Zachodni" );
    prop( d, "Radzymińska", Color::LIGHTBLUE, 100, 50, 50, Rent{ 6, 30, 90, 270, 400, 550 } );
    prop( d, "Jagiellońska", Color::LIGHTBLUE, 100, 50, 50, Rent{ 6, 30, 90, 270, 400, 550 } );
    prop( d, "Targowa", Color::LIGHTBLUE, 120, 60, 50, Rent{ 8, 40, 100, 300, 450, 600 } );
    prop( d, "Płowiecka", Color::PINK, 140, 70, 100, Rent{ 10, 50, 150, 450, 625, 750 } );
    util( d, "Elektrownia" );
    prop( d, "Marsa", Color::PINK, 140, 70, 100, Rent{ 10, 50, 150, 450, 625, 750 } );
    prop( d, "Grochowska", Color::PINK, 160, 80, 100, Rent{ 12, 60, 180, 500, 700, 900 } );
    rail( d, "Dworzec Gdański" );
    prop( d, "Obozowa", Color::ORANGE, 180, 90, 100, Rent{ 14, 70, 200, 550, 750, 950 } );
    prop( d, "Górczewska", Color::ORANGE, 180, 90, 100, Rent{ 14, 70, 200, 550, 750, 950 } );
    prop( d, "Wolska", Color::ORANGE, 200, 100, 100, Rent{ 16, 80, 220, 600, 800, 1000 } );
    prop( d, "Mickiewicza", Color::RED, 220, 110, 150, Rent{ 18, 90, 250, 700, 875, 1050 } );
    prop( d, "Słowackiego", Color::RED, 220, 110, 150, Rent{ 18, 90, 250, 700, 875, 1050 } );
    prop( d, "Plac Wilsona", Color::RED, 240, 120, 150, Rent{ 20, 100, 300, 750, 925, 1100 } );
    rail( d, "Dworzec Wschodni" );
    prop( d, "Świętokrzyska", Color::YELLOW, 260, 130, 150, Rent{ 22, 110, 330, 800, 975, 1150 } );
    prop( d, "Krakowskie Przedmieście", Color::YELLOW, 260, 130, 150, Rent{ 22, 110, 330, 800, 975, 1150 } );
    util( d, "Wodociągi" );
    prop( d, "Nowy Świat", Color::YELLOW, 280, 140, 150, Rent{ 24, 120, 360, 850, 1025, 1200 } );
    prop( d, "Plac Trzech Krzyży", Color::GREEN, 300, 150, 200, Rent{ 26, 130, 390, 900, 1100, 1275 } );
    prop( d, "Marszałkowska", Color::GREEN, 300, 150, 200, Rent{ 26, 130, 390, 900, 1100, 1275 } );
    prop( d, "Aleje Jerozolimskie", Color::GREEN, 320, 160, 200, Rent{ 28, 150, 450, 1000, 1200, 1400 } );
    rail( d, "Dworzec Centralny" );
    prop( d, "Belwederska", Color::DARKBLUE, 350, 175, 200, Rent{ 35, 175, 500, 1100, 1300, 1500 } );
    prop( d, "Aleje Ujazdowskie", Color::DARKBLUE, 400, 200, 200, Rent{ 50, 200, 600, 1400, 1700, 2000 } );
    return d;
}

vector<unique_ptr<Space>> getInitialBoard( const vector<unique_ptr<Card>>& d ) {
    vector<unique_ptr<Space>> b;
    spac( b, "Start", SpaceType::GO );
    spac( b, "Konopacka", SpaceType::PROPERTY, d[0].get() );
    spac( b, "Kasa społeczna", SpaceType::COMMUNITY_CHEST );
    spac( b, "Stalowa", SpaceType::PROPERTY, d[1].get() );
    spac( b, "Podatek dochodowy", SpaceType::TAX );
    spac( b, "Dworzec Zachodni", SpaceType::RAILWAY, d[2].get() );
    spac( b, "Radzymińska", SpaceType::PROPERTY, d[3].get() );
    spac( b, "Szansa", SpaceType::CHANCE );
    spac( b, "Jagiellońska", SpaceType::PROPERTY, d[4].get() );
    spac( b, "Targowa", SpaceType::PROPERTY, d[5].get() );
    spac( b, "Więzienie", SpaceType::JAIL );
    spac( b, "Płowiecka", SpaceType::PROPERTY, d[6].get() );
    spac( b, "Elektrownia", SpaceType::UTILITY, d[7].get() );
    spac( b, "Marsa", SpaceType::PROPERTY, d[8].get() );
    spac( b, "Grochowska", SpaceType::PROPERTY, d[9].get() );
    spac( b, "Dworzec Gdański", SpaceType::RAILWAY, d[10].get() );
    spac( b, "Obozowa", SpaceType::PROPERTY, d[11].get() );
    spac( b, "Kasa społeczna", SpaceType::COMMUNITY_CHEST );
    spac( b, "Górczewska", SpaceType::PROPERTY, d[12].get() );
    spac( b, "Wolska", SpaceType::PROPERTY, d[13].get() );
    spac( b, "Bezpłatny parking", SpaceType::FREE_PARKING );
    spac( b, "Mickiewicza", SpaceType::PROPERTY, d[14].get() );
    spac( b, "Szansa", SpaceType::CHANCE );
    spac( b, "Słowackiego", SpaceType::PROPERTY, d[15].get() );
    spac( b, "Plac Wilsona", SpaceType::PROPERTY, d[16].get() );
    spac( b, "Dworzec Wschodni", SpaceType::RAILWAY, d[17].get() );
    spac( b, "Świętokrzyska", SpaceType::PROPERTY, d[18].get() );
    spac( b, "Krakowskie Przedmieście", SpaceType::PROPERTY, d[19].get() );
    spac( b, "Wodociągi", SpaceType::UTILITY, d[20].get() );
    spac( b, "Nowy Świat", SpaceType::PROPERTY, d[21].get() );
    spac( b, "Idź do więzienia", SpaceType::GO_TO_JAIL );
    spac( b, "Plac Trzech Krzyży", SpaceType::PROPERTY, d[22].get() );
    spac( b, "Marszałkowska", SpaceType::PROPERTY, d[23].get() );
    spac( b, "Kasa społeczna", SpaceType::COMMUNITY_CHEST );
    spac( b, "Aleje Jerozolimskie", SpaceType::PROPERTY, d[24].get() );
    spac( b, "Dworzec Centralny", SpaceType::RAILWAY, d[25].get() );
    spac( b, "Szansa", SpaceType::CHANCE );
    spac( b, "Belwederska", SpaceType::PROPERTY, d[26].get() );
    spac( b, "Domiar podatkowy", SpaceType::TAX );
    spac( b, "Aleje Ujazdowskie", SpaceType::PROPERTY, d[27].get() );
    return b;
}

vector<unique_ptr<RandomCard>> getInitialChanceDeck() {
    vector<unique_ptr<RandomCard>> d;
    chan( d, "Rusz się na Aleje Ujazdowskie", LAMBDA { g->movePlayerToSpace( p, "Aleje Ujazdowskie" ); } );
    chan( d, "Rusz się na Start", LAMBDA { g->movePlayerToSpace( p, "Start" ); } );
    chan( d, "Rusz się na Nowy Świat", LAMBDA { g->movePlayerToSpace( p, "Nowy Świat" ); } );
    chan( d, "Rusz się na Płowiecką", LAMBDA { g->movePlayerToSpace( p, "Płowiecka" ); } );
    chan( d, "Rusz się na najbliższy Dworzec", LAMBDA { g->movePlayerToNearest( p, SpaceType::RAILWAY ); } );
    chan( d, "Rusz się na najbliższy Dworzec", LAMBDA { g->movePlayerToNearest( p, SpaceType::RAILWAY ); } );
    chan( d, "Rusz się do najbliższego Zakładu", LAMBDA { g->movePlayerToNearest( p, SpaceType::UTILITY ); } );
    chan( d, "Bank wypłaca ci dywidendę o wartości $50", LAMBDA { p->addCash( 50 ); } );
    chan( d, "Ucieczka z więzienia", LAMBDA { p->addJailFreeCard(); } );
    chan( d, "Cofnij się o 3 pola", LAMBDA { g->movePlayer( p, -3 ); } );
    chan( d, "Idź do więzienia", LAMBDA { g->movePlayerToJail( p ); } );
    chan( d, "Remont. Zapłać $25 za każdy dom i $100 za każdy hotel", LAMBDA { p->payRepairCosts( 25, 100 ); } );
    chan( d, "Speeding fine $15", LAMBDA { p->subtractCash( 15 ); } );
    chan( d, "Take a trip to Reading Railroad", LAMBDA { g->movePlayerToSpace( p, "Dworzec Zachodni" ); } );
    chan( d, "Mianowano cię Prezesem Zarządu. Zapłać każdemu graczowi $50", LAMBDA { g->payEachPlayer( p, 50 ); } );
    chan( d, "Twój kredyt budowlany się kończy. Pobierz $150", LAMBDA { p->addCash( 150 ); } );
    return d;
}

vector<unique_ptr<RandomCard>> getInitialCommunityChestDeck() {
    vector<unique_ptr<RandomCard>> d;
    ches( d, "Rusz się na Start", LAMBDA { g->movePlayerToSpace( p, "Start" ); } );
    ches( d, "Błąd banku na twoją korzyść. Otrzymujesz $200", LAMBDA { p->addCash( 200 ); } );
    ches( d, "Opłata za wizytę lekarską. Zapłać $50", LAMBDA { p->subtractCash( 50 ); } );
    ches( d, "Ze sprzedaży akcji otrzymujesz $50", LAMBDA { p->addCash( 50 ); } );
    ches( d, "Ucieczka z więzienia", LAMBDA { p->addJailFreeCard(); } );
    ches( d, "Idź do więzienia", LAMBDA { g->movePlayerToJail( p ); } );
    ches( d, "Fundusz wakacyjny dobiega końca. Pobierz $100", LAMBDA { p->addCash( 100 ); } );
    ches( d, "Zwrot podatku dochodowego. Pobierz $20", LAMBDA { p->addCash( 20 ); } );
    ches( d, "Masz urodziny. Zbierz od każdego gracza $10", LAMBDA { g->collectFromEachPlayer( p, 10 ); } );
    ches( d, "Ubezpieczenie zdrowotne dobiega końca. Pobierz $100", LAMBDA { p->addCash( 100 ); } );
    ches( d, "Zapłać $100 opłaty za szpital", LAMBDA { p->subtractCash( 100 ); } );
    ches( d, "Zapłać $50 czesnego", LAMBDA { p->subtractCash( 50 ); } );
    ches( d, "Odbierz $25 opłaty konsultacyjnej", LAMBDA { p->addCash( 25 ); } );
    ches( d, "Naprawa ulicy. Zapłać $40 za każdy dom i $115 za każdy hotel", LAMBDA { p->payRepairCosts( 40, 115 ); } );
    ches( d, "Zajmujesz drugie miejsce w konkursie piękności. Pobierz $10", LAMBDA { p->addCash( 10 ); } );
    ches( d, "Odziedziczasz $100", LAMBDA { p->addCash( 100 ); } );
    return d;
}
