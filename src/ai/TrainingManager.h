#ifndef TRAININGMANAGER_H
#define TRAININGMANAGER_H

#include <vector>
#include <mlpack.hpp>

const std::string POLICY_FILE = "policy/policy.bin";

// Author: Jan Mizera
// Training helper for AIPlayer using PPO algorithm.

// Hyperparameters_____________________________________________________________________________________________________
const int BATCH_SIZE = 64;
const double GAMMA = 0.99;
const double LAMBDA = 0.95;
const double CLIP_EPSILON = 0.2;
const double LEARNING_RATE = 0.001;
const int PPO_EPOCHS = 4;

struct Experience {
    arma::colvec state_;
    arma::colvec actionMask_;
    unsigned int action_;
    double reward_;
    arma::colvec nextState_;
    bool done_;

    double oldPolicyProb_ = 0.0;
};

class TrainingManager {
public:
    TrainingManager();
    unsigned int predictAction( const arma::colvec& state, const arma::colvec& action_mask );
    void recordExperience( const arma::colvec& state,
                           const arma::colvec& action_mask,
                           unsigned int action,
                           double reward,
                           const arma::colvec& next_state,
                           bool done );
    bool savePolicyNetwork( const std::string& filename );
    bool loadPolicyNetwork( const std::string& filename );

private:
    std::vector<Experience> experience_buffer_;

    mlpack::ann::FFN<mlpack::ann::NegativeLogLikelihood, mlpack::ann::HeInitialization> policy_network_;
    mlpack::ann::FFN<mlpack::ann::MeanSquaredError, mlpack::ann::HeInitialization> value_network_;

    ens::Adam optimizer_;

    arma::colvec actor_m_;
    arma::colvec actor_v_;
    size_t actor_time_step_ = 0;

    void updateActorWeights( arma::mat& params, const arma::mat& gradients );

    void trainStep();
    double computeValue( const arma::colvec& state );
    void computeGAE( const std::vector<double>& values,
                     const std::vector<double>& next_values,
                     std::vector<double>& advantages,
                     std::vector<double>& returns );
};

#endif  // TRAININGMANAGER_H