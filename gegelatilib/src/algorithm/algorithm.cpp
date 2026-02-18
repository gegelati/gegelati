
#include "algorithm/algorithm.h"

// Declaration of static agent ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t ALGORITHM_COUNTER_ID = 0;

uint64_t Algorithm::Algorithm::incrementeCounter()
{
    return ALGORITHM_COUNTER_ID++;
}

uint64_t Algorithm::Algorithm::getAlgorithmIDCounter()
{
    return ALGORITHM_COUNTER_ID;
}

void Algorithm::Algorithm::resetAlgorithmIDCounter()
{
    ALGORITHM_COUNTER_ID = 0;
}

uint64_t Algorithm::Algorithm::getAlgorithmID() const
{
    return this->algorithmID;
}

void Algorithm::Algorithm::setAlgorithmID(uint64_t newID)
{
    this->algorithmID = newID;

    // Update the ID counter if needed
    if (newID >= ALGORITHM_COUNTER_ID) {
        ALGORITHM_COUNTER_ID = newID + 1;
    }
}

Algorithm::Algorithm& Algorithm::Algorithm::getSubAlgorithm(uint64_t algorithmID)
{
    for (auto& subAlgorithm : this->subAlgorithms) {
        if (subAlgorithm->getAlgorithmID() == algorithmID) {
            return *subAlgorithm;
        }
    }
    throw std::runtime_error("No sub-algorithm with id " + std::to_string(algorithmID) + " found.");
}
const Algorithm::Algorithm& Algorithm::Algorithm::cGetSubAlgorithm(uint64_t algorithmID) const
{
    for (auto& subAlgorithm : this->subAlgorithms) {
        if (subAlgorithm->getAlgorithmID() == algorithmID) {
            return *subAlgorithm;
        }
    }
    throw std::runtime_error("No sub-algorithm with id " + std::to_string(algorithmID) + " found.");
}

void Algorithm::Algorithm::addSubAlgorithm(const Algorithm& subAlgorithm)
{
    // Throw if the sub-algorithm is already present
    for (auto& existingSubAlgorithm : this->subAlgorithms) {
        if (existingSubAlgorithm->getAlgorithmID() == subAlgorithm.getAlgorithmID()) {
            throw std::runtime_error("Algorithm::addSubAlgorithm: Sub-algorithm with name " + std::to_string(subAlgorithm.getAlgorithmID()) + " is already present.");
        }
    }
    this->subAlgorithms.push_back(subAlgorithm.copy());
}

void Algorithm::Algorithm::addAggregatedAlgorithm(const Algorithm& aggregatedAlgorithm){

    // Check that the types are the same
    if(typeid(*this) != typeid(aggregatedAlgorithm)){
        throw std::runtime_error("Algorithm::addAlgorithmAccess: Cannot add access to an algorithm of a different type.");
    }

    this->aggregatedAlgorithms.push_back(aggregatedAlgorithm);
}

const EvoGraph::Graph& Algorithm::Algorithm::getGraph() const
{
    return *this->graph;
}

const Algorithm::AgentManager& Algorithm::Algorithm::getManagerCst() const
{
    return *this->manager;
}

const Selector::Selector& Algorithm::Algorithm::getSelectorCst() const
{
    return *this->selector;
}

Algorithm::AgentManager& Algorithm::Algorithm::getManager()
{
    return *this->manager;
}

Selector::Selector& Algorithm::Algorithm::getSelector()
{
    return *this->selector;
}

Algorithm::Mutator& Algorithm::Algorithm::getMutator()
{
    return *this->mutator;
}

std::vector<std::reference_wrapper<const Algorithm::Algorithm>> Algorithm::Algorithm::getSubAlgorithms() const
{
    std::vector<std::reference_wrapper<const Algorithm>> subAlgorithmsRef;
    for(const auto& subAlgorithm : this->subAlgorithms){
        subAlgorithmsRef.push_back(*subAlgorithm);
    }
    return subAlgorithmsRef;
}

size_t Algorithm::Algorithm::getNbAgents() const
{
    return this->manager->getAgents().size();   
}

const std::vector<std::reference_wrapper<const Algorithm::Agent>> Algorithm::Algorithm::getAgents() const
{
    return this->manager->getAgents();
}

bool Algorithm::Algorithm::containsAgent(const Agent& agent) const
{
    return this->manager->containsAgent(agent);
}

void Algorithm::Algorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) {
    /*This method is not abstract, it is not necessary for an algorithm to have sub algorithms*/
}

void Algorithm::Algorithm::initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->outputs = std::make_unique<Output::OutputHandler>(outputs);
    this->graph = graph;

    this->initManager();

    this->selector = std::move(Selector::selectorFactory(*this->manager, this->params));

    this->initMutator();

    this->initSubAlgorithms(rng, outputs, dataSource, graph);

    // Add the aggregated algorithm
    for(const auto& aggregatedAlgorithm: this->aggregatedAlgorithms){
        this->manager->addAggregatedManager(aggregatedAlgorithm.get().getManagerCst());
    }

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();

}

void Algorithm::Algorithm::clearAlgorithm()
{
    for(const auto& subAlgorithm: subAlgorithms){
        subAlgorithm->clearAlgorithm();
    }
    this->manager->clearAgents(*this->graph);
    this->mutator = nullptr;
    this->selector = nullptr;
    this->manager = nullptr;
}

void Algorithm::Algorithm::initPopulation(RNG::RNG& rng)
{
    // Initialize a random population
    this->mutator->initRandomPopulation(*this->graph, *this->manager,
                                        this->params, rng);
}

void Algorithm::Algorithm::populate(RNG::RNG& rng, size_t maxNbThreads)
{
    this->mutator->mutatePopulation(*this->graph, *this->manager, this->params, rng, maxNbThreads);
    this->selector->updateAfterPopulate(*graph);

    this->clearUnusedSubAgents();
}

std::map<uint64_t, std::set<std::reference_wrapper<const Algorithm::Agent>>> Algorithm::Algorithm::getUsedSubAgents() const {
    // By default, return an empty map, meaning that no sub-agent is used by the algorithm.
    if(this->subAlgorithms.size() > 0){
        throw std::runtime_error("Algorithm::getUsedSubAgents: This method should be override by algorithms with sub-algorithms to return the used sub-agents.");
    }
    return std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>();
}

void Algorithm::Algorithm::clearUnusedSubAgents() {
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents = this->getUsedSubAgents();
    
    for(const auto& subAlgorithm: subAlgorithms){
        subAlgorithm->clearUnusedSubAgents();

        auto subAlgorithmAgents = subAlgorithm->getManager().getAgents();
        for(const Agent& agent: subAlgorithmAgents){
            if(usedSubAgents[subAlgorithm->getAlgorithmID()].find(agent) == usedSubAgents[subAlgorithm->getAlgorithmID()].end()){
                subAlgorithm->manager->deleteAgent(agent, *this->graph);
            }
        }
    }
}



std::shared_ptr<Algorithm::Job> Algorithm::Algorithm::createJob(const Agent& agent, Learn::LearningMode mode,  RNG::RNG& rng, int idx) const
{
    if (!this->containsAgent(agent)) {
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with an invalid agent or an agent not belonging to this algorithm.");
    }

    return std::make_shared<Job>(agent, idx);
}

void Algorithm::Algorithm::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // By default, do nothing
}


bool Algorithm::operator<(const Algorithm& a, const Algorithm& b)
{
    return a.getAlgorithmID() < b.getAlgorithmID();
}
bool Algorithm::operator==(const Algorithm& a, const Algorithm& b)
{
    return a.getAlgorithmID() == b.getAlgorithmID();
}
bool Algorithm::operator!=(const Algorithm& a, const Algorithm& b)
{
    return a.getAlgorithmID() != b.getAlgorithmID();
}