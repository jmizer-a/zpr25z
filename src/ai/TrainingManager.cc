#include "TrainingManager.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include "ai/Codec.h"

using namespace std;

// Author: Jan Mizera
// Training helper for AIPlayer using PPO algorithm.
// Implements experience recording, action prediction, and network training.

// Initializes the neural networks and optimizer_______________________________________________________________________

TrainingManager::TrainingManager() : optimizer_( LEARNING_RATE, BATCH_SIZE, 0.9, 0.999, 1e-8, 100000, 1e-5, true ) {
    value_network_.Add<mlpack::ann::Linear>( 128 );
    value_network_.Add<mlpack::ann::ReLU>();
    value_network_.Add<mlpack::ann::Linear>( 64 );
    value_network_.Add<mlpack::ann::ReLU>();
    value_network_.Add<mlpack::ann::Linear>( 1 );

    policy_network_.Add<mlpack::ann::Linear>( 128 );
    policy_network_.Add<mlpack::ann::ReLU>();
    policy_network_.Add<mlpack::ann::Linear>( 64 );
    policy_network_.Add<mlpack::ann::ReLU>();
    policy_network_.Add<mlpack::ann::Linear>( ACTION_SPACE_SIZE );
    policy_network_.Add<mlpack::ann::LogSoftMax>();

    actor_time_step_ = 0;
}

// Updates actor network weights using Adam optimizer__________________________________________________________________

void TrainingManager::updateActorWeights( arma::mat& params, const arma::mat& gradients ) {
    if( actor_m_.n_elem == 0 ) {
        actor_m_ = arma::zeros<arma::colvec>( params.n_elem );
        actor_v_ = arma::zeros<arma::colvec>( params.n_elem );
    }
    const double beta1 = 0.9;
    const double beta2 = 0.999;
    const double epsilon = 1e-8;

    ++actor_time_step_;

    actor_m_ = beta1 * actor_m_ + ( 1 - beta1 ) * gradients;
    actor_v_ = beta2 * actor_v_ + ( 1 - beta2 ) * ( gradients % gradients );

    double bias_correction1 = 1.0 - pow( beta1, actor_time_step_ );
    arma::colvec m_hat = actor_m_ / bias_correction1;

    double bias_correction2 = 1.0 - pow( beta2, actor_time_step_ );
    arma::colvec v_hat = actor_v_ / bias_correction2;

    arma::colvec denominator = arma::sqrt( v_hat ) + epsilon;

    params -= LEARNING_RATE * ( m_hat / denominator );
}

// Predicts the next action based on the current state and action mask_________________________________________________

unsigned int TrainingManager::predictAction( const arma::colvec& state, const arma::colvec& action_mask ) {
    arma::mat input = state;
    arma::mat output;

    policy_network_.Predict( input, output );

    arma::vec probs = arma::exp( output.col( 0 ) );
    probs %= action_mask;

    double sum_probs = arma::sum( probs );
    if( sum_probs < 1e-9 ) {
        probs = action_mask;
        sum_probs = arma::sum( probs );
    }
    probs /= sum_probs;

    double r = static_cast<double>( rand() ) / RAND_MAX;
    double cumulative = 0.0;
    for( unsigned int i = 0; i < probs.n_elem; ++i ) {
        cumulative += probs( i );
        if( r < cumulative ) {
            return i;
        }
    }
    return probs.n_elem - 1;
}

// Records experience and triggers training if buffer is full__________________________________________________________

void TrainingManager::recordExperience( const arma::colvec& state,
                                        const arma::colvec& action_mask,
                                        unsigned int action,
                                        double reward,
                                        const arma::colvec& next_state,
                                        bool done ) {
    Experience exp{ state, action_mask, action, reward, next_state, done };
    experience_buffer_.push_back( exp );
    if( experience_buffer_.size() >= BATCH_SIZE ) {
        trainStep();
        experience_buffer_.clear();
    }
}

// Estimates the value of a given state using the critic network_______________________________________________________

double TrainingManager::computeValue( const arma::colvec& state ) {
    arma::mat input = state;
    arma::mat output;
    value_network_.Predict( input, output );
    return output( 0, 0 );
}

// Computes Generalized Advantage Estimation and target returns________________________________________________________

void TrainingManager::computeGAE( const vector<double>& values,
                                  const vector<double>& next_values,
                                  vector<double>& advantages,
                                  vector<double>& returns ) {
    double gae = 0.0;
    for( int t = static_cast<int>( values.size() ) - 1; t >= 0; --t ) {
        double delta = experience_buffer_[t].reward_ + GAMMA * next_values[t] - values[t];
        gae = delta + GAMMA * LAMBDA * gae;
        if( experience_buffer_[t].done_ ) {
            gae = delta;
        }
        advantages[t] = gae;
        returns[t] = advantages[t] + values[t];
    }
}

// Executes the PPO training loop: computes advantages, updates critic and optimizes policy network____________________

void TrainingManager::trainStep() {
    size_t n = experience_buffer_.size();

    vector<double> values( n );
    vector<double> next_values( n );
    vector<double> advantages( n );
    vector<double> returns( n );
    vector<double> old_log_probs( n );

    for( size_t i = 0; i < n; ++i ) {
        values[i] = computeValue( experience_buffer_[i].state_ );
        next_values[i] = ( experience_buffer_[i].done_ ) ? 0.0 : computeValue( experience_buffer_[i].nextState_ );

        arma::mat output;
        policy_network_.Predict( experience_buffer_[i].state_, output );
        old_log_probs[i] = output( experience_buffer_[i].action_, 0 );
    }

    computeGAE( values, next_values, advantages, returns );

    double mean_adv = 0.0;
    double std_adv = 0.0;

    for( double a : advantages ) {
        mean_adv += a;
    }
    mean_adv /= n;

    for( double a : advantages ) {
        std_adv += ( a - mean_adv ) * ( a - mean_adv );
    }
    std_adv = sqrt( std_adv / ( n + 1e-8 ) );

    for( size_t i = 0; i < n; ++i ) {
        advantages[i] = ( advantages[i] - mean_adv ) / ( std_adv + 1e-8 );
    }

    for( int epoch = 0; epoch < PPO_EPOCHS; ++epoch ) {
        for( size_t i = 0; i < n; ++i ) {
            arma::mat target( 1, 1 );
            target( 0, 0 ) = returns[i];
            value_network_.Train( experience_buffer_[i].state_, target, optimizer_ );
        }

        for( size_t i = 0; i < n; ++i ) {
            const auto& expr = experience_buffer_[i];

            arma::mat output_logits;
            policy_network_.Forward( expr.state_, output_logits );

            double current_log_prob = output_logits( expr.action_, 0 );
            double ratio = exp( current_log_prob - old_log_probs[i] );
            double adv = advantages[i];
            double clipped_ratio = clamp( ratio, 1.0 - CLIP_EPSILON, 1.0 + CLIP_EPSILON );

            double grad_output = 0.0;
            bool is_clipped = ( ratio * adv ) > ( clipped_ratio * adv );

            if( !is_clipped ) {
                grad_output = -adv * ratio;
            }

            arma::mat error = arma::zeros( output_logits.n_rows, output_logits.n_cols );
            error( expr.action_, 0 ) = grad_output;

            double entropy_coef = 0.01;

            arma::vec probs = arma::exp( output_logits );

            for( unsigned int j = 0; j < output_logits.n_elem; ++j ) {
                double d_entropy = probs( j ) * ( 1.0 + output_logits( j ) );
                error( j, 0 ) += entropy_coef * d_entropy;
            }

            arma::mat gradients;
            policy_network_.Backward( expr.state_, error, gradients );

            arma::mat& params = policy_network_.Parameters();
            updateActorWeights( params, gradients );
        }
    }
}

// Saves and loads the policy network to/from a file___________________________________________________________________

bool TrainingManager::savePolicyNetwork( const string& filename ) {
    return mlpack::data::Save( filename, "policy_network", policy_network_, false );
}

bool TrainingManager::loadPolicyNetwork( const string& filename ) {
    return mlpack::data::Load( filename, "policy_network", policy_network_, false );
}