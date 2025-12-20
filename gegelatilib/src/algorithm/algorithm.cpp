
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
    this->subAlgorithms.push_back(subAlgorithm);
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

void Algorithm::Algorithm::init(RNG::RNG& rng, Learn::LearningEnvironment& le, std::shared_ptr<EvoGraph::Graph> graph)
{

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
        subAlgorithm->init(rng, le, graph);
        this->manager->addSubManager(subAlgorithm->getManager());
        this->mutator->addSubMutator(subAlgorithm->getMutator());

        // Initialize a random population
        this->mutator->initRandomPopulation(this->graph, this->manager,
                                            this->params, rng);

        this->mutator->mutatePopulation(this->graph, this->manager, this->selector, this->params, rng);


        // Clear the best agent in the selector
        this->selector->forgetPreviousResults();

    }
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

    // Before each agent evaluation, set a new seed for the archive in
    // TRAINING Mode Else, archiving should be deactivate anyway
    uint64_t archiveSeed = 0;
    if (mode ==Learn::LearningMode::TRAINING) {
        archiveSeed = rng.getUnsignedInt64(0, UINT64_MAX);
    }

    return std::make_shared<Job>(agent, this->manager, this->selector, archiveSeed, idx);
}

void Algorithm::Algorithm::activeJob(Job& job)
{
    /* Nothing to active for classic jobs */
}
