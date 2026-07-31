

#include "representation/atpg/atpgExecutionEngine.h"


void Representation::ATPG::ATPGExecutionEngine::setActionProgramExecutionEngine(std::unique_ptr<ExecutionEngine> actionProgramExecutionEngine){
    uint64_t representationID = actionProgramExecutionEngine->getRepresentationID();
    this->subExecutionEngines.insert({representationID, std::move(actionProgramExecutionEngine)});
    this->actionProgramExecutionEngine = this->subExecutionEngines.at(representationID).get();
}

std::vector<double> Representation::ATPG::ATPGExecutionEngine::execute()
{
    const Representation::TPG::TPGAgent& tpgAgent = dynamic_cast<const TPG::TPGAgent&>((*this->executedAgent).get());
    if(&tpgAgent == nullptr){
        throw std::runtime_error("Representation::ATPG::ATPGExecutionEngine::execute trying to execute an agent which is not a TPG agent");
    }
    std::reference_wrapper<const EvoGraph::Vertex> currentVertex = tpgAgent.getVertex();

    // Browse the TPG until vertex with an agent of the actionProgram Representation name is reached
    while (!currentVertex.get().hasProgram() || currentVertex.get().getProgram().getRepresentationID() != this->actionProgramExecutionEngine->getRepresentationID()) {

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
    this->actionValues = this->actionProgramExecutionEngine->execute();

    if(this->outputs.sizeContinuous() == 0){
        return this->actionValues;
    } else {
        /// TODO SET ACTIVATION FUNCTION
        return Utils::ActivationFunctions::scaleOutputValues(this->actionValues, this->outputs, Utils::ActivationFunction::TANH);
    }
}