
#include "algorithm/algorithm.h"


std::shared_ptr<Algorithm::Algorithm> Algorithm::Algorithm::getSubAlgorithm(std::string nameAlgorithm)
{
    for (auto& subAlgorithm : this->subAlgorithms) {
        if (subAlgorithm->getAlgorithmName() == nameAlgorithm) {
            return subAlgorithm;
        }
    }
    throw std::runtime_error("No sub-algorithm with name " + nameAlgorithm + " found.");
}
std::shared_ptr<const Algorithm::Algorithm> Algorithm::Algorithm::cGetSubAlgorithm(std::string nameAlgorithm) const
{
    for (auto& subAlgorithm : this->subAlgorithms) {
        if (subAlgorithm->getAlgorithmName() == nameAlgorithm) {
            return subAlgorithm;
        }
    }
    throw std::runtime_error("No sub-algorithm with name " + nameAlgorithm + " found.");
}

void Algorithm::Algorithm::addSubAlgorithm(std::shared_ptr<Algorithm> subAlgorithm)
{
    // Throw if the sub-algorithm is already present
    for (auto& existingSubAlgorithm : this->subAlgorithms) {
        if (existingSubAlgorithm->getAlgorithmName() == subAlgorithm->getAlgorithmName()) {
            throw std::runtime_error("Algorithm::addSubAlgorithm: Sub-algorithm with name " + subAlgorithm->getAlgorithmName() + " is already present.");
        }
    }
    this->subAlgorithms.push_back(subAlgorithm);
}

std::shared_ptr<const EvoGraph::Graph> Algorithm::Algorithm::getGraph() const
{
    return this->graph;
}

std::shared_ptr<const Algorithm::AgentManager> Algorithm::Algorithm::getManagerCst() const
{
    return this->manager;
}

std::shared_ptr<const Selector::Selector> Algorithm::Algorithm::getSelectorCst() const
{
    return this->selector;
}

std::shared_ptr<Algorithm::AgentManager> Algorithm::Algorithm::getManager()
{
    return this->manager;
}

std::shared_ptr<Selector::Selector> Algorithm::Algorithm::getSelector()
{
    return this->selector;
}

std::shared_ptr<Algorithm::Mutator> Algorithm::Algorithm::getMutator()
{
    return this->mutator;
}

size_t Algorithm::Algorithm::getNbAgents() const
{
    return this->manager->getAgents().size();   
}

const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::Algorithm::getAgents() const
{
    return this->manager->getAgents();
}

bool Algorithm::Algorithm::containsAgent(std::shared_ptr<const Agent> agent) const
{
    return this->manager->containsAgent(agent);
}

void Algorithm::Algorithm::initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->outputs = outputs;
    if(this->mutator == nullptr) {
        throw std::runtime_error("Mutator is not initialized in Algorithm " + this->algorithmName);
    }
    if(this->manager == nullptr) {
        throw std::runtime_error("Manager is not initialized in Algorithm " + this->algorithmName);
    }

    this->graph = graph;

    // Set the algorithm name to the components
    this->manager->setAlgorithmName(algorithmName);
    this->mutator->setAlgorithmName(algorithmName);
    this->selector = Selector::selectorFactory(this->manager, this->params);


    for (auto& subAlgorithm : this->subAlgorithms) {
        subAlgorithm->initAlgorithm(rng, outputs, dataSource, graph);
        this->manager->addSubManager(subAlgorithm->getManager());
        this->mutator->addSubMutator(subAlgorithm->getMutator());
    }

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();

}

void Algorithm::Algorithm::initPopulation(RNG::RNG& rng)
{
    // Initialize a random population
    this->mutator->initRandomPopulation(this->graph, this->manager,
                                        this->params, rng);
}

void Algorithm::Algorithm::populate(RNG::RNG& rng, size_t maxNbThreads)
{
    this->mutator->mutatePopulation(this->graph, this->manager, this->selector, this->params, rng, maxNbThreads);
}



std::shared_ptr<Algorithm::Job> Algorithm::Algorithm::createJob(std::shared_ptr<const Agent> agent, Learn::LearningMode mode,  RNG::RNG& rng, int idx) const
{
    if(agent == nullptr || !this->containsAgent(agent)){
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with a null agent or an agent not belonging to this algorithm.");
    }


    return std::make_shared<Job>(agent, idx);
}

void Algorithm::Algorithm::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // By default, do nothing
}

