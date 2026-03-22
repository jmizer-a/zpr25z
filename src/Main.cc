#include <memory>
#include <QApplication>
#include <vector>
#include "ai/AIPlayer.h"
#include "core/Card.h"
#include "core/Game.h"
#include "core/GameConfig.h"
#include "gui/GUIPlayer.h"
#include "gui/MainWindow.h"

using namespace std;

// Author: Jan Mizera & Jerzy Mossakowski
// Entries point for GUI Monopoly game with AI opponents.

int main( int argc, char* argv[] ) {
    QApplication app( argc, argv );

    MainWindow window;
    window.show();

    vector<unique_ptr<Player>> players;
    players.push_back( make_unique<GUIPlayer>( "Player 1", &window ) );
    TrainingManager trainer;
    if( !trainer.loadPolicyNetwork( POLICY_FILE ) ) {
        cerr << "Failed to load policy from " << POLICY_FILE << ". Train the model first.\n";
        return -1;
    }
    for( int i = 1; i < 4; ++i ) {
        auto ai_player = make_unique<AIPlayer>( "Player AI " + to_string( i ) );
        ai_player->setTrainer( &trainer );
        ai_player->setTrainingMode( false );
        players.push_back( move( ai_player ) );
    }

    vector<unique_ptr<Card>> deck = getInitialDeck();
    vector<unique_ptr<Space>> board = getInitialBoard( deck );
    vector<unique_ptr<RandomCard>> chance_deck = getInitialChanceDeck();
    vector<unique_ptr<RandomCard>> community_chest_deck = getInitialCommunityChestDeck();

    Game monopoly( move( players ), move( board ), move( deck ), move( chance_deck ), move( community_chest_deck ) );

    monopoly.run();

    Player* winner = monopoly.getWinner();
    window.displayWinner( winner );

    return 0;
}
