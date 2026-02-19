

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
    std::reference_wrapper<const EvoGraph::Vertex> currentVertex = tpgAgent.getVertex();

    // Browse the TPG until vertex with an agent of the actionProgram Algorithm name is reached
    while (!currentVertex.get().hasProgram() || currentVertex.get().getProgram().getAlgorithmID() != this->actionProgramExecutionEngine->getAlgorithmID()) {

        const EvoGraph::Team& teamVertex = dynamic_cast<const EvoGraph::Team&>(currentVertex.get());
        if(&teamVertex == nullptr){
            throw std::runtime_error("ATPGExecutionEngine:execute currentVertex to evaluate is not a team");
        }

        // Get the next edge
        const EvoGraph::Edge& edge = this->evaluateTeam(teamVertex);

        currentVertex = edge.getDestination();
        
    }

    // Set the progExecutionEngine to the program
    this->actionProgramExecutionEngine->setExecutedAgent(currentVertex.get().getProgram());

    // The action algorithm should already cast the action in the wanted range. 
    return this->actionProgramExecutionEngine->execute();
}