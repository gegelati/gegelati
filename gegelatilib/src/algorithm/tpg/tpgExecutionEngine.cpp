

#include "algorithm/tpg/tpgExecutionEngine.h"



double Algorithm::TPG::TPGExecutionEngine::evaluateEdge(const EvoGraph::Edge& edge)
{

    // Get the agent program
    std::shared_ptr<const Algorithm::Agent> program = edge.getProgram();

    // Set the progExecutionEngine to the program
    this->programExecutionEngine->setExecutedAgent(program);

    // Execute the program.
    double result = this->programExecutionEngine->execute().front();

    // Filter NaN results: replace with -inf
    result = (std::isnan(result)) ? -std::numeric_limits<double>::infinity()
                                  : result;

    // Put the result in the archive before returning it.
    /*if (this->archive != NULL) {
        this->archive->addRecording(&prog, progExecutionEngine.getDataSources(),
                                    result);
    }*/

    return result;
}

const EvoGraph::Edge& Algorithm::TPG::TPGExecutionEngine::evaluateTeam(const EvoGraph::Team& team)
{
    // Copy outgoing edge list
    const std::list<EvoGraph::Edge*>& outgoingEdges = team.getOutgoingEdges();

    // Evaluate all Edge
    // First
    EvoGraph::Edge* bestEdge = *outgoingEdges.begin();
    double bestBid = this->evaluateEdge(*bestEdge);
    // Others
    for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
         iter++) {
        EvoGraph::Edge* edge = *iter;
        double bid = this->evaluateEdge(*edge);
        if (bid >= bestBid) {
            bestEdge = edge;
            bestBid = bid;
        }
        else {
        }
    }

    return *bestEdge;
}


std::vector<double> Algorithm::TPG::TPGExecutionEngine::execute()
{
    auto tpgAgent = std::dynamic_pointer_cast<const TPGAgent>(this->executedAgent);
    if(tpgAgent == nullptr){
        throw std::runtime_error("Algorithm::TPG::TPGExecutionEngine::execute trying to execute an agent which is not a TPG agent");
    }
    const EvoGraph::Vertex* currentVertex = &tpgAgent->getVertex();
    const EvoGraph::Edge* edge = nullptr;

    std::vector<const EvoGraph::Vertex*> visitedVertices;
    visitedVertices.push_back(currentVertex);
    // Browse the TPG until a Action is reached.
    while (dynamic_cast<const EvoGraph::Team*>(currentVertex)) {
        // Get the next edge
        edge = &this->evaluateTeam(*(const EvoGraph::Team*)currentVertex);

        // update currentVertex and backup in visitedVertex.
        if (edge->getDestination() != nullptr) {
            currentVertex = edge->getDestination();
        }
        visitedVertices.push_back(currentVertex);
    }

    std::vector<double> actionID = {(double)dynamic_cast<const EvoGraph::Action*>(currentVertex)->getActionID()};
    return actionID;
}