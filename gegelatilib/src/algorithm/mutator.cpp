
#include "algorithm/mutator.h"

void Algorithm::Mutator::mutatePopulation(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::shared_ptr<Selector::Selector> selector,
    const Learn::LearningParameters& params,
    RNG::RNG& rng, size_t nbOutputs, uint64_t maxNbThread)
{

    // If the graph doesn't contain any clonable teams, call the init procedure.
    // (note that execution of this code is not a very good sign.. maybe an
    // exception would be more appropriate?)
    if (selector->updateContext().teamsClonable.size() <= 1) {
        initRandomPopulation(graph, manager, params, rng, nbOutputs);
        std::cerr<<"New population initialized during training because size was equal or below one"<<std::endl;
    } 
    const Selector::SelectionContext& context = selector->updateContext();
   
    std::vector<std::shared_ptr<const Algorithm::Agent>> subAgentsClonable1(
        context.agentsClonable);

    // Divide agents clonable into two subVector with half of the agents, randomly
    // selected.
    std::vector<std::shared_ptr<const Algorithm::Agent>> subAgentsClonable2;
    for (size_t idx = 0; idx < context.actionsClonable.size() / 2; idx++) {
        auto agent = subAgentsClonable1.at(
            rng.getUnsignedInt64(0, subAgentsClonable1.size() - 1));
        subAgentsClonable2.push_back(agent);
        std::swap(agent, subAgentsClonable1.back());
        subAgentsClonable1.pop_back();
    }

    // Agents newly created during the evolution that belong to another algorithm.
    std::vector<std::shared_ptr<const Agent>>  newSubAgents;

    
    // Create the new agents
    uint64_t nbAgentsToReach = manager->getAgents().size() + context.nbActionsToCreate;
    while (manager->getAgents().size() < nbAgentsToReach) {

        // Clone one random offspring.
        uint64_t clonedRootIndex1 =
            rng.getUnsignedInt64(0, subAgentsClonable1.size() - 1);

        std::vector<std::shared_ptr<const Algorithm::Agent>> offsprings;

        offsprings.push_back(manager->copyAgent(subAgentsClonable1.at(clonedRootIndex1), graph));

        // Be sure we have agents in both sub lists, and we still have at least
        // two agents to create
        if (subAgentsClonable2.size() > 0 &&
            manager->getAgents().size() < nbAgentsToReach - 1) {

            uint64_t clonedRootIndex2 =
                rng.getUnsignedInt64(0, subAgentsClonable2.size() - 1);

            // clone the offset
            offsprings.push_back(manager->copyAgent(subAgentsClonable2.at(clonedRootIndex2), graph));

            // Do the crossover over the childs
            this->crossoverAgents(offsprings, graph, manager, context, newSubAgents, params, rng);
        }

        // Do the mutation over the childs
        for (auto offspring : offsprings) {
            if (!offspring->isValid()) {
                manager->deleteAgent(offspring, graph);
            }
            else {
                // Apply mutations to the root and increase the number of roots
                this->mutateAgent(offspring, graph, manager, context, newSubAgents, params, rng);
            }
        }
    }

    selector->updateAfterPopulate();
}