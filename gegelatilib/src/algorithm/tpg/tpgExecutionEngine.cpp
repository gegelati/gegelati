

#include "algorithm/tpg/tpgExecutionEngine.h"


void Algorithm::TPG::TPGExecutionEngine::setArchive(Archive& archive)
{
    this->archive = &archive;
}
Archive& Algorithm::TPG::TPGExecutionEngine::getArchive()
{
    return *archive;
}

void Algorithm::TPG::TPGExecutionEngine::setProgramExecutionEngine(std::unique_ptr<ExecutionEngine> programExecutionEngine){
    std::string algorithmName = programExecutionEngine->getAlgorithmName();
    this->subExecutionEngines.insert({algorithmName, std::move(programExecutionEngine)});
    this->programExecutionEngine = this->subExecutionEngines.at(algorithmName).get();
}

void Algorithm::TPG::TPGExecutionEngine::setupJob(const Algorithm::Job& job)
{
    const TPGJob* tpgJob = dynamic_cast<const TPGJob*>(&job);
    if(tpgJob == nullptr){
        throw std::runtime_error("Algorithm::TPG::TPGExecutionEngine::setupJob trying to setup with a job which is not a TPGJob");
    }
    if(this->isTraining){
        this->setArchive(*tpgJob->getArchive());
    }
    this->setExecutedAgent(job.getAgent());
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
    if (this->isTraining && this->archive != nullptr) {
        this->archive->addRecording(edge.getProgram(), this->programExecutionEngine->getDataSources(),
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
    const Algorithm::TPG::TPGAgent& tpgAgent = dynamic_cast<const TPGAgent&>((*this->executedAgent).get());
    if(&tpgAgent == nullptr){
        throw std::runtime_error("Algorithm::TPG::TPGExecutionEngine::execute trying to execute an agent which is not a TPG agent");
    }
    std::shared_ptr<const EvoGraph::Vertex> currentVertex = tpgAgent.getVertex();
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
        return {(double)std::dynamic_pointer_cast<const EvoGraph::Action>(currentVertex)->getActionID()};
    } else {
        /// TODO SET ACTIVATION FUNCTION
        return Utils::ActivationFunctions::scaleOutputValues(actionValues, this->outputs, Utils::ActivationFunction::TANH);
    }
}


const std::vector<std::reference_wrapper<const Data::DataHandler>>& Algorithm::TPG::TPGExecutionEngine::getDataSources() const
{
    return this->dataSources;
}