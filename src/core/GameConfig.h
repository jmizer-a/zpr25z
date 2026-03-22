#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <memory>
#include <vector>
#include "Game.h"

// Author: Jerzy Mossakowski
// Responsible for configuration of the game, with board, cards, and players.

std::vector<std::unique_ptr<Card>> getInitialDeck();
std::vector<std::unique_ptr<Space>> getInitialBoard( const std::vector<std::unique_ptr<Card>>& deck );
std::vector<std::unique_ptr<RandomCard>> getInitialChanceDeck();
std::vector<std::unique_ptr<RandomCard>> getInitialCommunityChestDeck();

#endif  // GAMECONFIG_H
