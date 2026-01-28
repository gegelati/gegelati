

#include "algorithm/atpg/atpgExecutionEngine.h"


void Algorithm::ATPG::ATPGExecutionEngine::evaluateAction(const EvoGraph::Action& action)
{
    // Set the progExecutionEngine to the program
    this->actionProgramExecutionEngine->setExecutedAgent(action.getProgram());

    // Execute the program.
    this->actionValues = this->actionProgramExecutionEngine->execute();
}


std::vector<double> Algorithm::ATPG::ATPGExecutionEngine::execute()
{
    auto tpgAgent = std::dynamic_pointer_cast<const ATPGAgent>(this->executedAgent);
    if(tpgAgent == nullptr){
        throw std::runtime_error("Algorithm::ATPG::ATPGExecutionEngine::execute trying to execute an agent which is not a ATPG agent");
    }
    std::shared_ptr<const EvoGraph::Vertex> currentVertex = tpgAgent->getVertex();
    std::shared_ptr<const EvoGraph::Edge> edge = nullptr;

    std::vector<std::shared_ptr<const EvoGraph::Vertex>> visitedVertices;
    visitedVertices.push_back(currentVertex);
    // Browse the TPG until a Action is reached.
    while (auto teamVertex = std::dynamic_pointer_cast<const EvoGraph::Team>(currentVertex)) {
        // Get the next edge
        edge = this->evaluateTeam(*teamVertex);

        // update currentVertex and backup in visitedVertex.
        if (edge->getDestination() != nullptr) {
            currentVertex = edge->getDestination();
        }
        visitedVertices.push_back(currentVertex);
    }

    if(auto action = std::dynamic_pointer_cast<const EvoGraph::Action>(currentVertex)){
        // Evaluate the action program to get the action value.
        this->evaluateAction(*action);
    } else {
        throw std::runtime_error("Algorithm::ATPG::ATPGExecutionEngine::execute: Reached a non-action vertex at the end of the execution.");
    }

    // The action algorithm should already cast the action in the wanted range. 
    return this->actionValues;
}