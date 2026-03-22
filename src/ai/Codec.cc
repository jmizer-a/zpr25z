#include "Codec.h"

using namespace std;

// Author: Jan Mizera
// Implements encoding the state of the Monopoly game and generating action masks for the AI players.
// Translates the classes defined in the core module into numerical representations.

/// Encode the current game state into a fixed-size vector_____________________________________________________________

arma::colvec Codec::encodeGameState( const vector<unique_ptr<Player>>& players,
                                     const vector<unique_ptr<Space>>& board,
                                     const unordered_map<Player*, unsigned int>& positions,
                                     Player* me,
                                     GamePhase phase ) {
    arma::colvec observation( STATE_SIZE, arma::fill::zeros );
    int offset = 0;

    arma::colvec phase_one_hot( PHASE, arma::fill::zeros );
    phase_one_hot( static_cast<int>( phase ) ) = 1.0;
    observation.subvec( offset, offset + PHASE - 1 ) = phase_one_hot;
    offset += PHASE;

    for( size_t i = 0; i < players.size(); ++i ) {
        observation( offset + i ) = ( players[i].get() == me ) ? 1.0 : 0.0;
    }
    offset += ACTIVE_PLAYER_IDX;

    int per_player_size = POSITION + CASH + JAIL_STATUS + JAIL_PROGRESS + GAME_STATUS + GET_OUT_OF_JAIL_CARDS;

    for( size_t i = 0; i < players.size(); ++i ) {
        Player* player = players[i].get();
        unsigned int position = positions.at( player );
        int current_idx = offset + ( static_cast<int>( i ) * per_player_size );

        observation( current_idx ) = static_cast<double>( position ) / 39.0;
        ++current_idx;
        observation( current_idx ) = static_cast<double>( player->getCash() ) / 1500.0;
        ++current_idx;
        observation( current_idx ) = ( player->getTurnsInJail() > 0 ) ? 1.0 : 0.0;
        ++current_idx;
        observation( current_idx ) = static_cast<double>( player->getTurnsInJail() ) / 3.0;
        ++current_idx;
        observation( current_idx ) = player->isLost() ? 1.0 : 0.0;
        ++current_idx;
        unsigned int jail_free_cards = 0;
        for( Card* card : player->getOwnedCards() ) {
            if( card->getName() == JAIL_FREE_CARD_NAME ) {
                ++jail_free_cards;
            }
        }
        observation( current_idx ) = static_cast<double>( jail_free_cards ) / 2.0;
    }
    offset += PLAYERS * per_player_size;

    for( const auto& space : board ) {
        Card* card = space->getAssociatedCard();
        PurchasableCard* p_card = dynamic_cast<PurchasableCard*>( card );

        if( p_card ) {
            Player* owner = p_card->getOwner();
            for( size_t i = 0; i < players.size(); ++i ) {
                if( owner == players[i].get() ) {
                    observation( offset + i ) = 1.0;
                }
            }
            if( owner == nullptr ) {
                observation( offset + PLAYERS ) = 1.0;
            }
            offset += OWNERSHIP;

            Property* prop = dynamic_cast<Property*>( p_card );
            if( prop ) {
                observation( offset ) = 1.0;
            } else if( p_card->getName().find( "Elektrownia" ) != string::npos ||
                       p_card->getName().find( "Wodociągi" ) != string::npos ) {
                observation( offset + 1 ) = 1.0;
            } else {
                observation( offset + 2 ) = 1.0;
            }
            offset += TYPE;

            observation( offset ) = p_card->isMortgaged() ? 1.0 : 0.0;
            ++offset;

            if( prop ) {
                observation( offset ) = static_cast<double>( prop->getHouseCount() ) / 4.0;
                ++offset;
                observation( offset ) = prop->getHotelCount() > 0 ? 1.0 : 0.0;
                ++offset;
                observation( offset ) = prop->isMonopoly() ? 1.0 : 0.0;
                ++offset;
            } else {
                offset += HOUSE_COUNT + HOTEL_COUNT + MONOPOLY_STATUS;
            }
        }
    }
    return observation;
}

// Generate an appropriate action mask based on the current game state and game phase__________________________________

arma::colvec Codec::getActionMask( const vector<unique_ptr<Player>>& players,
                                   const vector<unique_ptr<Space>>& board,
                                   const unordered_map<Player*, unsigned int>& positions,
                                   Player* me,
                                   GamePhase phase,
                                   unsigned int extra_context ) {
    arma::colvec action_mask( ACTION_SPACE_SIZE, arma::fill::zeros );
    switch( phase ) {
        case GamePhase::MY_TURN: {
            action_mask( ROLL_DICE_ID ) = 1.0;
            int street_idx = 0;
            int purchasable_idx = 0;
            for( const auto& space : board ) {
                Card* card = space->getAssociatedCard();
                if( auto* p_card = dynamic_cast<PurchasableCard*>( card ) ) {
                    bool is_owner = ( p_card->getOwner() == me );

                    bool can_mortgage = is_owner && !p_card->isMortgaged();

                    int lifted_mortgage_cost = static_cast<int>( p_card->getMortgageValue() * 1.1 );
                    bool can_unmortgage =
                        is_owner && p_card->isMortgaged() && ( me->getCash() >= lifted_mortgage_cost );

                    if( auto* prop = dynamic_cast<Property*>( p_card ) ) {
                        if( prop->getHouseCount() > 0 || prop->getHotelCount() > 0 ) {
                            can_mortgage = false;
                        }

                        unsigned int prop_lvl = prop->getHotelCount() > 0 ? 5 : prop->getHouseCount();
                        bool even_build_rule = true;
                        if( is_owner && prop->isMonopoly() ) {
                            unsigned int min_houses = 5;
                            for( const auto& s_check : board ) {
                                if( auto* p_check = dynamic_cast<Property*>( s_check->getAssociatedCard() ) ) {
                                    if( p_check->getColor() == prop->getColor() ) {
                                        unsigned int h = p_check->getHotelCount() > 0 ? 5 : p_check->getHouseCount();
                                        if( h < min_houses ) {
                                            min_houses = h;
                                        }
                                    }
                                }
                            }
                            if( prop_lvl > min_houses ) {
                                even_build_rule = false;
                            }
                        }

                        bool can_afford_build = me->getCash() >= prop->getHouseCost();

                        if( is_owner && prop->isMonopoly() && prop_lvl < 5 && even_build_rule && can_afford_build ) {
                            action_mask( BUILD_HOUSE_ID_START + street_idx ) = 1.0;
                        }

                        if( is_owner && prop_lvl > 0 ) {
                            action_mask( SELL_HOUSE_ID_START + street_idx ) = 1.0;
                        }
                        ++street_idx;
                    }

                    if( can_mortgage ) {
                        action_mask( MORTGAGE_ID_START + purchasable_idx ) = 1.0;
                    }
                    if( can_unmortgage ) {
                        action_mask( UNMORTGAGE_ID_START + purchasable_idx ) = 1.0;
                    }

                    ++purchasable_idx;
                }
            }
            break;
        }
        case GamePhase::BUY_DECISION: {
            unsigned int my_pos = positions.at( me );
            Space* current_space = board[my_pos].get();
            Card* associated = current_space->getAssociatedCard();

            if( auto* p_card = dynamic_cast<PurchasableCard*>( associated ) ) {
                if( me->getCash() >= p_card->getPrice() ) {
                    action_mask( BUY_PROPERTY_ID ) = 1.0;
                }
            }
            action_mask( DECLINE_PROPERTY_ID ) = 1.0;
            break;
        }
        case GamePhase::BIDDING:
            if( extra_context + BID_INCREMENT <= me->getCash() ) {
                action_mask( AUCTION_BID_ID ) = 1.0;
            }
            action_mask( AUCTION_PASS_ID ) = 1.0;
            break;

        case GamePhase::JAIL_DECISION: {
            bool can_use_card = false;
            for( Card* card : me->getOwnedCards() ) {
                if( card->getName() == JAIL_FREE_CARD_NAME ) {
                    can_use_card = true;
                    break;
                }
            }
            if( can_use_card ) {
                action_mask( USE_GET_OUT_OF_JAIL_CARD_ID ) = 1.0;
            }
            action_mask( ROLL_JAIL_DICE_ID ) = 1.0;
            break;
        }
        case GamePhase::TRADE_RESPONSE:
            action_mask( TRADE_ACCEPT_ID ) = ( extra_context == 1 ) ? 1.0 : 0.0;
            action_mask( TRADE_DECLINE_ID ) = 1.0;
            break;

        case GamePhase::TRADE_PROPOSAL: {
            action_mask( TRADE_DECLINE_ID ) = 1.0;
            int prop_idx = 0;
            for( const auto& space : board ) {
                if( auto* target_card = dynamic_cast<PurchasableCard*>( space->getAssociatedCard() ) ) {
                    Player* owner = target_card->getOwner();
                    bool is_valid_target =
                        ( owner != nullptr ) && ( owner != me ) && ( !owner->isLost() ) &&
                        ( target_card->getPrice() <= static_cast<double>( me->getCash() ) * TRADE_MULTIPLIER );
                    if( is_valid_target ) {
                        action_mask( prop_idx + PROPOSE_TRADE_START_ID ) = 1.0;
                    }
                    prop_idx++;
                }
            }
            break;
        }
        default:;
    }

    return action_mask;
}