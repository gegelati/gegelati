

#include <iostream>
#include "gridworld.h"

void GridWorld::reset(size_t seed, Learn::LearningMode mode, uint16_t iterationNumber, uint64_t generationNumber){

    // Reset agent coordonate
    agentCoord = {0, 0};
    
    // Reset terminated and score
    terminated = false;
    score = 0.0;

    // Set data
    currentState.setDataAt(typeid(int), 0, (int)agentCoord[0] + 1);
    currentState.setDataAt(typeid(int), 1, (int)agentCoord[1] + 1);
}

bool GridWorld::positionAvailable(uint64_t pos_x, uint64_t pos_y){

    // position unavailable on axis x
    if(pos_x == size[0] || pos_x == -1){
        return false;
    }

    // position unavailable on axis x
    if(pos_y == size[1] || pos_y == -1){
        return false;
    }

    // position unavailable because tile is unavailable
    if (grid[pos_y][pos_x] == 3){
        return false;
    }

    // Else : position is available
    return true;

}

void GridWorld::doActions(std::vector<uint64_t> vectActionID){

    // vectActionID[0] is Left / None / Right for (0, 1, 2)
    // vectActionID[1] is Up / None / Down for (0, 1, 2)
    if(positionAvailable(agentCoord[0] - (vectActionID[0] - (uint64_t)1), agentCoord[1]  - (vectActionID[1] - (uint64_t)1))){
        agentCoord[0] -= (vectActionID[0] - (uint64_t)1);
        agentCoord[1] -= (vectActionID[1] - (uint64_t)1);
    }

    // Reward is always -1 except when an output is reached
    double reward = -1;

    if(grid[agentCoord[1]][agentCoord[0]] == 1){
        // good output reached
        terminated = true;
        reward = 100;
    } else if(grid[agentCoord[1]][agentCoord[0]] == 2){
        // Bad output reached
        terminated = true;
        reward = -100;
    }

    // update score
    score += reward;

    // Set data
    currentState.setDataAt(typeid(int), 0, (int)agentCoord[0] + 1);
    currentState.setDataAt(typeid(int), 1, (int)agentCoord[1] + 1);
}

bool GridWorld::isTerminal() const{
    return terminated;
}

double GridWorld::getScore() const {
    return score;
}

std::vector<std::reference_wrapper<const Data::DataHandler>> GridWorld::getDataSources()
{
	auto result = std::vector<std::reference_wrapper<const Data::DataHandler>>();
	result.push_back(this->currentState);
	return result;
}

Learn::LearningEnvironment* GridWorld::clone() const
{
	return new GridWorld(*this);
}

bool GridWorld::isCopyable() const
{
	return true;
}