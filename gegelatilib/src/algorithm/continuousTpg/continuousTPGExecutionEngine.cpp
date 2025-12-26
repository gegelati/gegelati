
/*#include "algorithm/continuousTpg/continuousTPGExecutionEngine.h"

double Algorithm::ContinuousTPG::ContinuousTPGExecutionEngine::evaluateEdge(const EvoGraph::Edge& edge)
{


    // Set the progExecutionEngine to the program
    this->programExecutionEngine->setExecutedAgent(edge.getProgram());

    this->lastValues = this->programExecutionEngine->execute();

    // Execute the program.
    double result = lastValues.front();

    // Filter NaN results: replace with -inf
    result = (std::isnan(result)) ? -std::numeric_limits<double>::infinity()
                                  : result;


    // Put the result in the archive before returning it.
    if (this->isTraining) {
        this->archive.get().addRecording(*edge.getProgram(), this->getDataSources(),
                                    result);
    }

    return result;
}

std::shared_ptr<const EvoGraph::Edge> Algorithm::ContinuousTPG::ContinuousTPGExecutionEngine::evaluateTeam(const EvoGraph::Team& team)
{
    // Copy outgoing edge list
    const auto& outgoingEdges = team.getOutgoingEdges();

    // Evaluate all Edge
    // First
    std::shared_ptr<const EvoGraph::Edge> bestEdge = *outgoingEdges.begin();
    double bestBid = this->evaluateEdge(*bestEdge);

    this->actionValues.clear();
    this->actionValues.insert(this->lastValues.begin() + 1, this->lastValues.end(), this->actionValues.end());

    // Others
    for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
         iter++) {
        std::shared_ptr<const EvoGraph::Edge> edge = *iter;
        double bid = this->evaluateEdge(*edge);
        if (bid >= bestBid) {
            bestEdge = edge;
            bestBid = bid;
            this->actionValues.clear();
            this->actionValues.insert(this->lastValues.begin() + 1, this->lastValues.end(), this->actionValues.end());
        }
    }

    return bestEdge;
}


std::vector<double> Algorithm::ContinuousTPG::ContinuousTPGExecutionEngine::execute()
{
    auto tpgAgent = std::dynamic_pointer_cast<const TPG::TPGAgent>(this->executedAgent);
    if(tpgAgent == nullptr){
        throw std::runtime_error("Algorithm::TPG::TPGExecutionEngine::execute trying to execute an agent which is not a TPG agent");
    }
    std::shared_ptr<const EvoGraph::Vertex> currentVertex = tpgAgent->getVertex();
    std::shared_ptr<const EvoGraph::Edge> edge = nullptr;

    if(std::dynamic_pointer_cast<const EvoGraph::Action>(currentVertex) != nullptr){
        throw std::runtime_error("Action root can not exist with continuous TPG execution engine");
    }

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

    return this->actionValues;
}*/