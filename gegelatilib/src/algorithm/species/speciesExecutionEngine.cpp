

#include "algorithm/species/speciesExecutionEngine.h"


void Algorithm::Species::SpeciesExecutionEngine::setArchive(Archive& archive)
{
    this->archive = &archive;
}
Archive& Algorithm::Species::SpeciesExecutionEngine::getArchive()
{
    return *archive;
}

void Algorithm::Species::SpeciesExecutionEngine::setProgramExecutionEngine(std::unique_ptr<ExecutionEngine> programExecutionEngine){
    uint64_t algorithmID = programExecutionEngine->getAlgorithmID();
    this->subExecutionEngines.insert({algorithmID, std::move(programExecutionEngine)});
    this->programExecutionEngine = this->subExecutionEngines.at(algorithmID).get();
}

void Algorithm::Species::SpeciesExecutionEngine::setupJob(const Algorithm::Job& job)
{
    const SpeciesJob* speciesJob = dynamic_cast<const SpeciesJob*>(&job);
    if(speciesJob == nullptr){
        throw std::runtime_error("Algorithm::Species::SpeciesExecutionEngine::setupJob trying to setup with a job which is not a SpeciesJob");
    }
    if(this->isTraining){
        this->setArchive(speciesJob->getArchive());
    }
    this->setExecutedAgent(job.getAgent());
}

double Algorithm::Species::SpeciesExecutionEngine::evaluateEdge(const EvoGraph::Edge& edge)
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

void Algorithm::Species::SpeciesExecutionEngine::setContinuousActionValues()
{
    if(this->outputs.sizeContinuous() != 0){
        this->actionValues.clear();
        this->actionValues.insert(this->actionValues.begin(), this->lastValues.begin() + 1, this->lastValues.end());
    }
}

const EvoGraph::Edge& Algorithm::Species::SpeciesExecutionEngine::evaluateTeam(const EvoGraph::Team& team)
{
    // Copy outgoing edge list
    const auto& outgoingEdges = team.getOutgoingEdges();

    // Evaluate all Edge
    // First
    std::reference_wrapper<const EvoGraph::Edge> bestEdge = *outgoingEdges.begin();
    double bestBid = this->evaluateEdge(bestEdge);
    this->setContinuousActionValues();

    // Others
    for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
         iter++) {
        const EvoGraph::Edge& edge = *iter;
        double bid = this->evaluateEdge(edge);
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


std::vector<double> Algorithm::Species::SpeciesExecutionEngine::execute()
{
    const Algorithm::Species::SpeciesAgent& speciesAgent = dynamic_cast<const SpeciesAgent&>((*this->executedAgent).get());
    if(&speciesAgent == nullptr){
        throw std::runtime_error("Algorithm::Species::SpeciesExecutionEngine::execute trying to execute an agent which is not a Species agent");
    }
    std::reference_wrapper<const EvoGraph::Vertex> currentVertex = speciesAgent.getVertex();

    // Browse the Species until a Action is reached.
    while (auto teamVertex = dynamic_cast<const EvoGraph::Team*>(&currentVertex.get())) {
        // Get the next edge
        const EvoGraph::Edge& edge = this->evaluateTeam(*teamVertex);

        // update currentVertex and backup in visitedVertex.
        currentVertex = edge.getDestination();
    }

    if(this->outputs.sizeContinuous() == 0){
        return {(double)dynamic_cast<const EvoGraph::Action*>(&currentVertex.get())->getActionID()};
    } else {
        /// TODO SET ACTIVATION FUNCTION
        return Utils::ActivationFunctions::scaleOutputValues(actionValues, this->outputs, Utils::ActivationFunction::TANH);
    }
}


const std::vector<std::reference_wrapper<const Data::DataHandler>>& Algorithm::Species::SpeciesExecutionEngine::getDataSources() const
{
    return this->dataSources;
}