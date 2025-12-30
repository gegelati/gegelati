

#include "algorithm/tpg/tpgExecutionEngine.h"


void Algorithm::TPG::TPGExecutionEngine::setArchive(Archive& archive)
{
    this->archive = archive;
}
Archive& Algorithm::TPG::TPGExecutionEngine::getArchive()
{
    return archive;
}


double Algorithm::TPG::TPGExecutionEngine::evaluateEdge(const EvoGraph::Edge& edge)
{
    // Set the progExecutionEngine to the program
    this->programExecutionEngine->setExecutedAgent(edge.getProgram());

    // Execute the program.
    this->lastValues = this->programExecutionEngine->execute();
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

void Algorithm::TPG::TPGExecutionEngine::setContinuousActionValues()
{
    if(this->outputs.sizeContinuous() != 0){
        this->actionValues.clear();
        this->actionValues.insert(this->actionValues.begin(), this->lastValues.begin() + 1, this->lastValues.end());
    }
}

std::shared_ptr<const EvoGraph::Edge> Algorithm::TPG::TPGExecutionEngine::evaluateTeam(const EvoGraph::Team& team)
{
    // Copy outgoing edge list
    const auto& outgoingEdges = team.getOutgoingEdges();

    // Evaluate all Edge
    // First
    std::shared_ptr<const EvoGraph::Edge> bestEdge = *outgoingEdges.begin();
    double bestBid = this->evaluateEdge(*bestEdge);
    this->setContinuousActionValues();

    // Others
    for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
         iter++) {
        std::shared_ptr<const EvoGraph::Edge> edge = *iter;
        double bid = this->evaluateEdge(*edge);
        if (bid >= bestBid) {
            bestEdge = edge;
            bestBid = bid;
            this->setContinuousActionValues();
        }
        else {
        }
    }

    return bestEdge;
}


std::vector<double> Algorithm::TPG::TPGExecutionEngine::execute()
{
    auto tpgAgent = std::dynamic_pointer_cast<const TPGAgent>(this->executedAgent);
    if(tpgAgent == nullptr){
        throw std::runtime_error("Algorithm::TPG::TPGExecutionEngine::execute trying to execute an agent which is not a TPG agent");
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

    if(this->outputs.sizeContinuous() == 0){
        actionValues = {(double)std::dynamic_pointer_cast<const EvoGraph::Action>(currentVertex)->getActionID()};
    }
    return actionValues;
}


const std::vector<std::reference_wrapper<const Data::DataHandler>>& Algorithm::TPG::TPGExecutionEngine::getDataSources() const
{
    return this->programExecutionEngine->getDataSources();
}