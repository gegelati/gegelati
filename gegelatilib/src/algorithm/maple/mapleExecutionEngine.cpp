

#include "algorithm/maple/mapleExecutionEngine.h"

std::vector<double> Algorithm::Maple::MapleExecutionEngine::execute()
{
    auto mapleAgent = std::dynamic_pointer_cast<const MapleAgent>(this->executedAgent);
    if(mapleAgent == nullptr){
        throw std::runtime_error("Algorithm::Maple::MapleExecutionEngine::execute trying to execute an agent which is not a Maple agent");
    }
    std::shared_ptr<const EvoGraph::Action> actionVertex = std::dynamic_pointer_cast<const EvoGraph::Action>(mapleAgent->getVertex());
    if(actionVertex == nullptr){
        throw std::runtime_error("Algorithm::Maple::MapleExecutionEngine::execute trying to execute a Maple agent which does not represent an action vertex");
    }

    this->actionValues.resize(this->outputs.size(), 0.0);
    for(auto edge : actionVertex->getOutgoingEdges()){

        // Convert edge to actionEdge
        auto actionEdge = std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(edge);
        if(actionEdge == nullptr){
            throw std::runtime_error("Algorithm::Maple::MapleExecutionEngine::execute: an outgoing edge of an action vertex is not an action edge.");
        }

        // Set action value for the action class
        this->actionValues[actionEdge->getActionClass()] = this->evaluateEdge(*edge);
    }


    if(this->outputs.sizeContinuous() == 0){
        Output::convertContinuousToDiscreteOutputs(this->actionValues, this->outputs);
        return this->actionValues;
    } else {
        /// TODO SET ACTIVATION FUNCTION
        return Utils::ActivationFunctions::scaleOutputValues(this->actionValues, this->outputs, Utils::ActivationFunction::TANH);
    }
}