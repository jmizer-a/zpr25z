#include <fstream>
#include <iostream>
#include "ai/AIPlayer.h"
#include "ai/TrainingManager.h"
#include "core/Card.h"
#include "core/Game.h"
#include "core/GameConfig.h"
#include "core/Player.h"
#include "core/Space.h"

using namespace std;

// Author: Jan Mizera
// Performs training of a PPO model for the Monopoly game using MARL.
// Saves the resulting policy to the file specified in TrainingManager.h.
// Has a separate executable target in CMakeLists.txt.

const int PRINT_INTERVAL = 1;

int main() {
    cout << "Initializing PPO Training Manager...\n";
    TrainingManager trainer;

    int total_episodes;

    cout << "Enter number of training episodes:\n";
    cin >> total_episodes;
    cout << "Starting training for " << total_episodes << " episodes...\n";
    for( int episode = 1; episode <= total_episodes; ++episode ) {
        vector<unique_ptr<Player>> players;
        for( int i = 0; i < 4; ++i ) {
            auto ai_player = make_unique<AIPlayer>( "AI Player " + to_string( i ) );
            ai_player->setTrainer( &trainer );
            players.push_back( move( ai_player ) );
        }

        auto deck = getInitialDeck();
        auto chance_deck = getInitialChanceDeck();
        auto community_chest_deck = getInitialCommunityChestDeck();
        auto board = getInitialBoard( deck );

        Game game( move( players ), move( board ), move( deck ), move( chance_deck ), move( community_chest_deck ) );

        game.run();

        const auto& game_players = game.getPlayers();

        bool is_draw = false;
        unsigned int active_count = 0;
        for( const auto& p : game_players ) {
            if( !p->isLost() )
                active_count++;
        }
        if( active_count != 1 ) {
            is_draw = true;
        }
        for( size_t i = 0; i < game_players.size(); ++i ) {
            AIPlayer* ai = dynamic_cast<AIPlayer*>( game_players[i].get() );

            if( ai ) {
                ai->notifyGameEnd( is_draw );
            }
        }

        if( episode % PRINT_INTERVAL == 0 ) {
            cout << "Episode " << episode << "/" << total_episodes << " complete.\n";
        }
    }
    cout << "Training Complete.\n";
    bool save_success = trainer.savePolicyNetwork( POLICY_FILE );
    if( save_success ) {
        cout << "Trained policy saved to " << POLICY_FILE << ".\n";
    } else {
        cout << "Failed to save trained policy to " << POLICY_FILE << ".\n";
    }
    cout << "============================================\n";

    return 0;
}