

#include "representation/tpg/tpgExecutionEngine.h"


void Representation::TPG::TPGExecutionEngine::setArchive(TPGArchive& archive)
{
    this->archive = archive;
}
std::optional<std::reference_wrapper<Representation::TPG::TPGArchive>> Representation::TPG::TPGExecutionEngine::getArchive()
{
    return archive;
}

void Representation::TPG::TPGExecutionEngine::setProgramExecutionEngine(std::unique_ptr<ExecutionEngine> programExecutionEngine){
    uint64_t representationID = programExecutionEngine->getRepresentationID();
    this->subExecutionEngines.insert({representationID, std::move(programExecutionEngine)});
    this->programExecutionEngineID = representationID;
}

void Representation::TPG::TPGExecutionEngine::setupJob(const Representation::Job& job)
{
    const TPGJob* tpgJob = dynamic_cast<const TPGJob*>(&job);
    if(tpgJob == nullptr){
        throw std::runtime_error("Representation::TPG::TPGExecutionEngine::setupJob trying to setup with a job which is not a TPGJob");
    }
    if(this->isTraining){
        this->setArchive(tpgJob->getArchive());
    }
    this->setExecutedAgent(job.getAgent());
}

double Representation::TPG::TPGExecutionEngine::evaluateEdge(const EvoGraph::Edge& edge)
{
    // Set the progExecutionEngine to the program
    this->getProgramExecutionEngine().setExecutedAgent(edge.getProgram());

    // Execute the program.
    this->lastValues = this->getProgramExecutionEngine().execute();
    double result = lastValues.front();

    // Filter NaN results: replace with -inf
    result = (std::isnan(result)) ? -std::numeric_limits<double>::infinity()
                                  : result;

    // Put the result in the archive before returning it.
    if (this->isTraining && this->archive) {
        this->archive->get().addRecording(edge.getProgram(), this->getProgramExecutionEngine().getDataSources(),
                                    result);
    }
    return result;
}

void Representation::TPG::TPGExecutionEngine::setContinuousActionValues()
{
    if(this->outputs.sizeContinuous() != 0){
        this->actionValues.clear();
        this->actionValues.insert(this->actionValues.begin(), this->lastValues.begin() + 1, this->lastValues.end());
    }
}

const EvoGraph::Edge& Representation::TPG::TPGExecutionEngine::evaluateTeam(const EvoGraph::Team& team)
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


std::vector<double> Representation::TPG::TPGExecutionEngine::execute()
{
    const Representation::TPG::TPGAgent& tpgAgent = dynamic_cast<const TPGAgent&>((*this->executedAgent).get());
    if(&tpgAgent == nullptr){
        throw std::runtime_error("Representation::TPG::TPGExecutionEngine::execute trying to execute an agent which is not a TPG agent");
    }
    std::reference_wrapper<const EvoGraph::Vertex> currentVertex = tpgAgent.getVertex();

    // Browse the TPG until a Action is reached.
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


const std::vector<std::reference_wrapper<const Data::DataHandler>>& Representation::TPG::TPGExecutionEngine::getDataSources() const
{
    return this->dataSources;
}