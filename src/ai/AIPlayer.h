#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <mlpack.hpp>
#include "ai/Codec.h"
#include "ai/TrainingManager.h"
#include "core/Player.h"

// Author: Jan Mizera
// Implements an AI player that uses the Proximal Policy Optimization (PPO) algorithm.
// It acts as a proxy for a trained PPO model to make decisions in the Monopoly game.

class AIPlayer : public Player {
public:
    AIPlayer( std::string name ) : Player( std::move( name ) ), prev_wealth_( STARTING_CASH ) {}

    DiceResult takeTurn() override;
    PlayerDecision decideBuyOrAuction( const PurchasableCard& card ) override;
    unsigned int decideBid( const PurchasableCard& card, unsigned int current_bid ) override;
    bool useGetOutOfJailCard() override;
    DiceResult rollDice() const override;
    void informOfRandomCard( const RandomCard& card ) override;
    bool wantsToTrade() override;
    TradeOffer tradeOffer() override;
    bool decideOnTrade( TradeOffer trade_offer ) override;

    void setTrainer( TrainingManager* trainer );
    double calculateReward();

    void notifyGameEnd( bool is_draw );
    void setTrainingMode( bool is_training );

protected:
    virtual unsigned int getDecisionFromBrain( GamePhase phase, unsigned int extra_context = 0 );
    unsigned int getDecisionFromBrain( const arma::colvec& state, const arma::colvec& action_mask, double reward );

private:
    Property* getPropertyByIndex( int index ) const;
    PurchasableCard* getPurchasableByIndex( int index ) const;
    int getPropertyIndex( const PurchasableCard* target ) const;

    bool is_training_ = true;
    TrainingManager* trainer_ = nullptr;
    arma::colvec last_state_;
    arma::colvec last_mask_;
    unsigned int last_action_ = UNDEFINED_ID;
    bool has_pending_experience_ = false;
    double prev_wealth_;
    unsigned int pending_trade_id_ = UNDEFINED_ID;
};

#endif  // AIPLAYER_H