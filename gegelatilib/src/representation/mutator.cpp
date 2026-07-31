
#include "representation/mutator.h"

void Representation::Mutator::addSubMutator(Mutator& subMutator)
{
    this->subMutators.insert({subMutator.getRepresentationID(), subMutator});
}



Representation::Mutator& Representation::Mutator::getSubMutator(uint64_t representationID){
    auto it = this->subMutators.find(representationID);
    if(it == this->subMutators.end()){
        throw std::runtime_error("Representation::Mutator::getSubMutator subMutator not found for the specific name");
    }
    return it->second;
}

void Representation::Mutator::updateSpecificContext(
    EvoGraph::Graph& graph, Population& population,
    const RepresentationParameters& params,
    RNG::RNG& rng)
{
    this->currentContext = std::move(this->selector.get().updateContext());

    // Update the context of the subMutators
    for(auto subMutPair: this->subMutators){
        auto& subPopulation = population.getSubPopulation(subMutPair.first);
        subMutPair.second.get().updateSpecificContext(graph, subPopulation, params, rng);
    }
}

const Selector::SelectionContext& Representation::Mutator::getContext()
{
    return *this->currentContext;
}

std::vector<std::reference_wrapper<const EvoGraph::Action>> Representation::Mutator::initActionVertices(
    EvoGraph::Graph& graph, size_t nbActionVertices)
{
    // vector to store the actions
    std::vector<std::reference_wrapper<const EvoGraph::Action>> actions;

    // We want to fill actions with all thetd::vector<std::shared necessary actions.
    // Create the missing actions, and add the already existing actions.
    // PS:currentActions should be ordered by actionID, but in case it doesnt is, we check all the action vertices for each index.
    
    std::vector<std::reference_wrapper<const EvoGraph::Action>> currentActions = graph.getActions();
    for(size_t idx = 0; idx < nbActionVertices; idx++){
        // find the action if it exists.
        auto it = currentActions.begin();
        while(it != currentActions.end() && (*it).get().getActionID() != idx){
            it++;
        }

        // If the action is found, add it and erase it from the vector, else create the vertex.
        if(it == currentActions.end()){
            actions.push_back(graph.addNewAction(idx));
        } else {
            actions.push_back((*it));
            currentActions.erase(it);
        }
    }
    return actions;
}

const Representation::Individual& Representation::Mutator::initRandomAgent(
    EvoGraph::Graph& graph,
    Population& population,
    const RepresentationParameters& params, RNG::RNG& rng)
{
    const Representation::Individual& agent = population.createAgent(graph);
    this->initRandomSpecificAgent(agent, graph, population, params, rng);
    return agent;
}

void Representation::Mutator::mutatePopulation(
    EvoGraph::Graph& graph, Population& population,
    const RepresentationParameters& params,
    RNG::RNG& rng, uint64_t maxNbThreads)
{

    this->updateSpecificContext(graph, population, params, rng);
    // If the graph doesn't contain any clonable teams, call the init procedure.
    // (note that execution of this code is not a very good sign.. maybe an
    // exception would be more appropriate?)
    if (this->currentContext->agentsClonable.size() <= 1) {
        throw std::runtime_error("At least two agents should survive the selection");
    } 
    std::vector<std::reference_wrapper<const Representation::Individual>> subAgentsClonable1(
        this->currentContext->agentsClonable);

    // Divide agents clonable into two subVector with half of the agents, randomly
    // selected.
    std::vector<std::reference_wrapper<const Representation::Individual>> subAgentsClonable2;
    for (size_t idx = 0; idx < this->currentContext->agentsClonable.size() / 2; idx++) {
        auto agent = subAgentsClonable1.at(
            rng.getUnsignedInt64(0, subAgentsClonable1.size() - 1));
        subAgentsClonable2.push_back(agent);
        std::swap(agent, subAgentsClonable1.back());
        subAgentsClonable1.pop_back();
    }

    // Agents newly created during the evolution that belong to another representation.
    std::vector<std::reference_wrapper<const Individual>>  newSubAgents;

    
    // Create the new agents
    uint64_t nbAgentsToReach = population.getAgents().size() + this->currentContext->nbAgentsToCreate;
    while (population.getAgents().size() < nbAgentsToReach) {

        // Clone one random offspring.
        uint64_t clonedRootIndex1 =
            rng.getUnsignedInt64(0, subAgentsClonable1.size() - 1);

        std::vector<std::reference_wrapper<const Individual>> offsprings;

        offsprings.push_back(population.copyAgent(subAgentsClonable1.at(clonedRootIndex1), graph));

        // Be sure we have agents in both sub lists, and we still have at least
        // two agents to create
        if (subAgentsClonable2.size() > 0 &&
            population.getAgents().size() < nbAgentsToReach - 1) {

            uint64_t clonedRootIndex2 =
                rng.getUnsignedInt64(0, subAgentsClonable2.size() - 1);

            // clone the offset
            offsprings.push_back(population.copyAgent(subAgentsClonable2.at(clonedRootIndex2), graph));

            // Do the crossover over the childs
            this->crossoverAgents({offsprings.at(0), offsprings.at(1)}, graph, population, newSubAgents, params, rng);
        }

        // Do the mutation over the childs
        for (const Representation::Individual& offspring : offsprings) {
            if (!offspring.isValid()) {
                population.deleteAgent(offspring, graph);
            }
            else {
                // Apply mutations to the root and increase the number of roots
                this->mutateAgent(offspring, graph, population, newSubAgents,
                                  params, rng);
            }
        }

    }

    // Mutate the new subAgents.
    this->mutateSubAgents(newSubAgents, graph, population, params, rng, maxNbThreads);
}