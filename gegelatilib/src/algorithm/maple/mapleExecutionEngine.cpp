

#include "algorithm/maple/mapleExecutionEngine.h"

std::vector<double> Algorithm::Maple::MapleExecutionEngine::execute()
{
    const MapleAgent& mapleAgent = dynamic_cast<const MapleAgent&>((*this->executedAgent).get());
    if(&mapleAgent == nullptr){
        throw std::runtime_error("Algorithm::Maple::MapleExecutionEngine::execute trying to execute an agent which is not a Maple agent");
    }
    std::shared_ptr<const EvoGraph::Team> teamVertex = std::dynamic_pointer_cast<const EvoGraph::Team>(mapleAgent.getVertex());
    if(teamVertex == nullptr){
        throw std::runtime_error("Algorithm::Maple::MapleExecutionEngine::execute trying to execute a Maple agent which does not represent a team vertex");
    }

    this->actionValues.clear();
    this->actionValues.resize(this->outputs.size(), 0.0);
    for(auto edge : teamVertex->getOutgoingEdges()){

        // Convert destination to action
        auto action = std::dynamic_pointer_cast<const EvoGraph::Action>(edge->getDestination());
        if(action == nullptr){
            throw std::runtime_error("Algorithm::Maple::MapleExecutionEngine::execute: an outgoing edge does not lead to an action vertex.");
        }

        // Set action value for the action class
        this->actionValues[action->getActionID()] = this->evaluateEdge(*edge);
    }


    if(this->outputs.sizeContinuous() == 0){
        Output::convertContinuousToDiscreteOutputs(this->actionValues, this->outputs);
        return this->actionValues;
    } else {
        /// TODO SET ACTIVATION FUNCTION
        return Utils::ActivationFunctions::scaleOutputValues(this->actionValues, this->outputs, Utils::ActivationFunction::TANH);
    }
}