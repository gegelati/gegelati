

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

double Algorithm::Species::SpeciesExecutionEngine::evaluateProgram(const Agent& program)
{
    // Set the progExecutionEngine to the program
    this->programExecutionEngine->setExecutedAgent(program);

    // Execute the program.
    this->lastValues = this->programExecutionEngine->execute();
    double result = lastValues.front();

    // Filter NaN results: replace with -inf
    result = (std::isnan(result)) ? -std::numeric_limits<double>::infinity()
                                  : result;

    // Put the result in the archive before returning it.
    if (this->isTraining && this->archive != nullptr) {
        this->archive->addRecording(program, this->programExecutionEngine->getDataSources(),
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

void Algorithm::Species::SpeciesExecutionEngine::evaluateTeam(const EvoGraph::Vertex& vertex, size_t depth, const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Algorithm::Agent>>> & mapEdgeProgram)
{
    // Execute the team of the destination of the best edge.
    if (dynamic_cast<const EvoGraph::Team*>(&vertex) == nullptr) {
        throw std::runtime_error("SpeciesExecutionEngine::evaluateTeam: Evaluated vertex should be a team!");
    }

    // Copy outgoing edge list
    const auto& outgoingEdges = vertex.getOutgoingEdges();


    // Depth is odd, meaning we are on the "bid selection path"
    if(depth % 2 == 1) {
        // Evaluate all Edge
        // First
        std::reference_wrapper<const EvoGraph::Edge> bestEdge = *outgoingEdges.begin();
        double bestBid = this->evaluateProgram(*mapEdgeProgram.at(bestEdge));

        // Others
        for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
            iter++) {
            const EvoGraph::Edge& edge = *iter;
            double bid = this->evaluateProgram(*mapEdgeProgram.at(bestEdge));
            if (bid >= bestBid) {
                bestEdge = edge;
                bestBid = bid;
            }
        }
        this->evaluateTeam(bestEdge.get().getDestination(), depth + 1, mapEdgeProgram);


    // Depth is even, meaning we are on the "activation selection path"
    } else {
        for (const EvoGraph::Edge& edge: outgoingEdges) {
            // Edge is not in the map linking edge to program, it means its a connection edge without program between two teams. 
            if(mapEdgeProgram.find(edge) == mapEdgeProgram.end()) {
                // Execute the team of the destination of the best edge.
                this->evaluateTeam(edge.getDestination(), depth + 1, mapEdgeProgram);
            } else if (const EvoGraph::Action* action = dynamic_cast<const EvoGraph::Action*>(&edge.getDestination())) {
                // Set action value for the action class
                this->actionValues[action->getActionID()] = this->evaluateProgram(*mapEdgeProgram.at(edge));
            } else {
                throw std::runtime_error("SpeciesExecutionEngine::evaluateTeam: in even depth, edge should point to an action if it contains a program.");
            }
        }
        
    }
}


std::vector<double> Algorithm::Species::SpeciesExecutionEngine::execute()
{
    // At this point, the agent should be valid, and its edge should correspond to the species.
    const Algorithm::Species::SpeciesAgent* speciesAgent = dynamic_cast<const SpeciesAgent*>(&(*this->executedAgent).get());
    if(speciesAgent == nullptr){
        throw std::runtime_error("Algorithm::Species::SpeciesExecutionEngine::execute trying to execute an agent which is not a Species agent");
    }


    this->actionValues.clear();
    this->actionValues.resize(this->outputs.size(), 0.0);

    size_t currentMode = 0;
    this->evaluateTeam(this->rootVertex, 0, speciesAgent->getPrograms());

    
    if(this->outputs.sizeContinuous() == 0){
        Output::convertContinuousToDiscreteOutputs(this->actionValues, this->outputs);
        return this->actionValues;
    } else {
        /// TODO SET ACTIVATION FUNCTION
        return Utils::ActivationFunctions::scaleOutputValues(this->actionValues, this->outputs, Utils::ActivationFunction::TANH);
    }
}


const std::vector<std::reference_wrapper<const Data::DataHandler>>& Algorithm::Species::SpeciesExecutionEngine::getDataSources() const
{
    return this->dataSources;
}