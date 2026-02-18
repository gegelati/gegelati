

#include "algorithm/atpg/atpgExecutionEngine.h"


void Algorithm::ATPG::ATPGExecutionEngine::setActionProgramExecutionEngine(std::unique_ptr<ExecutionEngine> actionProgramExecutionEngine){
    uint64_t algorithmID = actionProgramExecutionEngine->getAlgorithmID();
    this->subExecutionEngines.insert({algorithmID, std::move(actionProgramExecutionEngine)});
    this->actionProgramExecutionEngine = this->subExecutionEngines.at(algorithmID).get();
}

std::vector<double> Algorithm::ATPG::ATPGExecutionEngine::execute()
{
    const Algorithm::TPG::TPGAgent& tpgAgent = dynamic_cast<const TPG::TPGAgent&>((*this->executedAgent).get());
    if(&tpgAgent == nullptr){
        throw std::runtime_error("Algorithm::ATPG::ATPGExecutionEngine::execute trying to execute an agent which is not a TPG agent");
    }
    std::shared_ptr<const EvoGraph::Vertex> currentVertex = tpgAgent.getVertex();
    std::shared_ptr<const EvoGraph::Edge> edge = nullptr;

    // Browse the TPG until vertex with an agent of the actionProgram Algorithm name is reached
    while (!currentVertex->hasProgram() || currentVertex->getProgram().getAlgorithmID() != this->actionProgramExecutionEngine->getAlgorithmID()) {

        auto teamVertex = std::dynamic_pointer_cast<const EvoGraph::Team>(currentVertex);
        if(teamVertex == nullptr){
            throw std::runtime_error("ATPGExecutionEngine:execute currentVertex to evaluate is not a team");
        }

        // Get the next edge
        edge = this->evaluateTeam(*teamVertex);

        currentVertex = edge->getDestination();
        
    }

    // Set the progExecutionEngine to the program
    this->actionProgramExecutionEngine->setExecutedAgent(currentVertex->getProgram());

    // The action algorithm should already cast the action in the wanted range. 
    return this->actionProgramExecutionEngine->execute();
}