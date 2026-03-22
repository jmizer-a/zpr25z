#ifndef CODEC_H
#define CODEC_H

#include <armadillo>
#include "core/Player.h"
#include "core/Space.h"

// Author: Jan Mizera
// Implements encoding the state of the Monopoly game and generating action masks for the AI players.

// Action Space________________________________________________________________________________________________________
const int ROLL_DICE_ID = 0;
const int BUY_PROPERTY_ID = 1;
const int DECLINE_PROPERTY_ID = 2;
const int BUILD_HOUSE_ID_START = 3;                         // IDs for building houses
const int SELL_HOUSE_ID_START = BUILD_HOUSE_ID_START + 22;  // IDs for selling houses
const int MORTGAGE_ID_START = SELL_HOUSE_ID_START + 22;     // IDs for mortgaging properties
const int UNMORTGAGE_ID_START = MORTGAGE_ID_START + 28;     // IDs for unmortgaging properties
const int AUCTION_BID_ID = UNMORTGAGE_ID_START + 28;
const int AUCTION_PASS_ID = AUCTION_BID_ID + 1;
const int USE_GET_OUT_OF_JAIL_CARD_ID = AUCTION_PASS_ID + 1;
const int ROLL_JAIL_DICE_ID = USE_GET_OUT_OF_JAIL_CARD_ID + 1;
const int TRADE_ACCEPT_ID = ROLL_JAIL_DICE_ID + 1;
const int TRADE_DECLINE_ID = TRADE_ACCEPT_ID + 1;
const int PROPOSE_TRADE_START_ID = TRADE_DECLINE_ID + 1;
const int PROPOSE_TRADE_END_ID = PROPOSE_TRADE_START_ID + 28;

const int ACTION_SPACE_SIZE = PROPOSE_TRADE_END_ID;

const int UNDEFINED_ID = ACTION_SPACE_SIZE + 1;

const int BID_INCREMENT = 50;
const double TRADE_MULTIPLIER = 1.2;

// State_______________________________________________________________________________________________________________
// Global context size
const int PHASE = 6;              // One-hot encoding for 6 phases
const int ACTIVE_PLAYER_IDX = 4;  // One-hot encoding for up to 4 players

// Per player context size
const int PLAYERS = 4;

const int POSITION = 1;               // Normalized position on the board (consider switching to cyclic?)
const int CASH = 1;                   // Normalized cash amount
const int JAIL_STATUS = 1;            // 0: not in jail, 1: in jail
const int JAIL_PROGRESS = 1;          // Normalized jail turns count
const int GAME_STATUS = 1;            // 0: active, 1: bankrupt
const int GET_OUT_OF_JAIL_CARDS = 1;  // Normalized count of Get Out of Jail Free cards

// Per property context size
const int PROPERTIES = 28;

const int OWNERSHIP = 5;        // One-hot encoding for up to 4 players + bank
const int TYPE = 3;             // One-hot encoding for property type (Set, Utility, Railroad)
const int MORTGAGE_STATUS = 1;  // 0: not mortgaged, 1: mortgaged
const int HOUSE_COUNT = 1;      // Normalized house count
const int HOTEL_COUNT = 1;      // 0: no hotel, 1: has hotel
const int MONOPOLY_STATUS = 1;  // 0: no monopoly, 1: has monopoly

const int AUCTIONS = 28;  // One per property, 0: no auction, 1: auction ongoing
const int BID = 1;        // Normalized current highest bid

const int CASH_DELTA = 1;             // Normalized cash delta in trade
const int IS_RECEIVING_TRADE = 1;     // 0: not receiving trade, 1: receiving trade
const int IS_SENDING_TRADE = 1;       // 0: not sending trade, 1: sending trade
const int PROPERTIES_OFFERED = 28;    // One per property, 0: not offered, 1: offered
const int PROPERTIES_REQUESTED = 28;  // One per property, 0: not requested, 1: requested

const int TRADE_CONTEXT_SIZE =
    CASH_DELTA + IS_RECEIVING_TRADE + IS_SENDING_TRADE + PROPERTIES_OFFERED + PROPERTIES_REQUESTED;

const int STATE_SIZE =
    PHASE + ACTIVE_PLAYER_IDX +
    PLAYERS * ( POSITION + CASH + JAIL_STATUS + JAIL_PROGRESS + GAME_STATUS + GET_OUT_OF_JAIL_CARDS ) +
    PROPERTIES * ( OWNERSHIP + TYPE + MORTGAGE_STATUS + HOUSE_COUNT + HOTEL_COUNT + MONOPOLY_STATUS ) + AUCTIONS + BID +
    TRADE_CONTEXT_SIZE;

const std::string JAIL_FREE_CARD_NAME = "Ucieczka z więzienia";

enum class GamePhase { MY_TURN, BUY_DECISION, BIDDING, JAIL_DECISION, TRADE_RESPONSE, TRADE_PROPOSAL };

class Codec {
public:
    static arma::colvec encodeGameState( const std::vector<std::unique_ptr<Player>>& players,
                                         const std::vector<std::unique_ptr<Space>>& board,
                                         const std::unordered_map<Player*, unsigned int>& positions,
                                         Player* me,
                                         GamePhase phase );
    static arma::colvec getActionMask( const std::vector<std::unique_ptr<Player>>& players,
                                       const std::vector<std::unique_ptr<Space>>& board,
                                       const std::unordered_map<Player*, unsigned int>& positions,
                                       Player* me,
                                       GamePhase phase,
                                       unsigned int current_bid = -1 );
};

#endif  // CODEC_H